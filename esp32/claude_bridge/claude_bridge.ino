// Claude bridge for the TI-84 Plus CE "welcome ~." program.
//
// The calculator sends one line per message over USB serial. This board
// asks the Claude API over Wi-Fi and sends back one line:
//   R:reply text\n   or   E:error text\n
//
// Needs: Arduino core for ESP32, and the ArduinoJson library (v7).
// Serial is the link to the calculator, so this sketch never prints debug text.

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ---- Fill these in ----
const char *WIFI_SSID         = "your-wifi-name";
const char *WIFI_PASSWORD     = "your-wifi-password";
const char *ANTHROPIC_API_KEY = "sk-ant-your-key";

// Root certificate for api.anthropic.com (see README.md for how to get it).
const char *ROOT_CA = R"(-----BEGIN CERTIFICATE-----
PASTE THE ROOT CERTIFICATE HERE
-----END CERTIFICATE-----
)";
// -----------------------

const char *API_URL = "https://api.anthropic.com/v1/messages";
const char *MODEL   = "claude-opus-5-5";

const char *SYSTEM_PROMPT =
    "You are Claude, chatting through a TI-84 Plus CE graphing calculator. "
    "Its screen fits about 26 characters per line and 10 lines. "
    "Reply in plain ASCII text only: no markdown, no emoji, no line breaks. "
    "Keep every reply under 200 characters. "
    "The user can only type capital letters, spaces and question marks, "
    "so read their messages generously.";

const long   BAUD_RATE    = 115200;
const size_t MAX_INPUT    = 64;
const size_t MAX_REPLY    = 300;  // the calculator holds up to 320 characters per line
const size_t MAX_HISTORY  = 10;   // messages kept for context (5 exchanges)

JsonDocument history;  // array of {role, content}
String inputLine;

// Turn Claude's UTF-8 text into plain ASCII the calculator font can draw
String toCalcText(const String &in)
{
    String out;
    for (size_t i = 0; i < in.length(); i++) {
        uint8_t c = in[i];
        if (c == '\n' || c == '\r' || c == '\t') {
            c = ' ';
        }
        if (c >= 0x20 && c < 0x7F) {
            if (!(c == ' ' && out.endsWith(" "))) {
                out += (char)c;
            }
        } else if (c == 0xE2 && i + 2 < in.length() && (uint8_t)in[i + 1] == 0x80) {
            uint8_t d = in[i + 2];
            if (d == 0x98 || d == 0x99) out += '\'';
            else if (d == 0x9C || d == 0x9D) out += '"';
            else if (d == 0x93 || d == 0x94) out += '-';
            else if (d == 0xA6) out += "...";
            i += 2;
        }
        // other non-ASCII bytes are dropped
    }
    out.trim();
    if (out.length() > MAX_REPLY) {
        int cut = out.lastIndexOf(' ', MAX_REPLY - 3);
        out = out.substring(0, cut > 0 ? cut : MAX_REPLY - 3) + "...";
    }
    return out;
}

void sendToCalc(char kind, const String &text)
{
    Serial.print(kind);
    Serial.print(':');
    Serial.print(text);
    Serial.print('\n');
}

void addToHistory(const char *role, const String &content)
{
    JsonArray msgs = history.as<JsonArray>();
    JsonObject m = msgs.add<JsonObject>();
    m["role"] = role;
    m["content"] = content;
    // Drop the oldest exchange so the history always starts with a user message
    while (msgs.size() > MAX_HISTORY) {
        msgs.remove(0);
        msgs.remove(0);
    }
}

void dropLastMessage()
{
    JsonArray msgs = history.as<JsonArray>();
    if (msgs.size() > 0) {
        msgs.remove(msgs.size() - 1);
    }
}

void askClaude(const String &text)
{
    if (WiFi.status() != WL_CONNECTED) {
        sendToCalc('E', "NO WI-FI. CHECK THE NETWORK NAME AND PASSWORD.");
        return;
    }

    addToHistory("user", text);

    JsonDocument req;
    req["model"] = MODEL;
    req["max_tokens"] = 2048;
    req["system"] = SYSTEM_PROMPT;
    req["output_config"]["effort"] = "low";
    req["fallbacks"] = "default";
    req["messages"] = history;
    String body;
    serializeJson(req, body);

    WiFiClientSecure client;
    client.setCACert(ROOT_CA);
    HTTPClient http;
    http.setTimeout(60000);
    http.begin(client, API_URL);
    http.addHeader("content-type", "application/json");
    http.addHeader("x-api-key", ANTHROPIC_API_KEY);
    http.addHeader("anthropic-version", "2026-10-21");
    http.addHeader("anthropic-beta", "server-side-fallback-2026-07-01");

    int status = http.POST(body);
    if (status != 200) {
        http.end();
        dropLastMessage();
        if (status == 401) sendToCalc('E', "BAD API KEY. CHECK ANTHROPIC_API_KEY.");
        else if (status == 429 || status == 529) sendToCalc('E', "CLAUDE IS BUSY. TRY AGAIN SOON.");
        else if (status < 0) sendToCalc('E', "COULD NOT REACH CLAUDE. CHECK WI-FI.");
        else sendToCalc('E', "CLAUDE API ERROR " + String(status) + ".");
        return;
    }

    // Keep only the fields we read, to save memory
    JsonDocument filter;
    filter["stop_reason"] = true;
    filter["content"][0]["type"] = true;
    filter["content"][0]["text"] = true;

    JsonDocument res;
    DeserializationError err = deserializeJson(res, http.getString(), DeserializationOption::Filter(filter));
    http.end();
    if (err) {
        dropLastMessage();
        sendToCalc('E', "COULD NOT READ CLAUDE'S REPLY.");
        return;
    }

    if (res["stop_reason"] == "refusal") {
        dropLastMessage();
        sendToCalc('E', "CLAUDE DECLINED TO ANSWER THAT.");
        return;
    }

    String reply;
    for (JsonObject block : res["content"].as<JsonArray>()) {
        if (block["type"] == "text") {
            reply += block["text"].as<const char *>();
        }
    }
    if (reply.length() == 0) {
        dropLastMessage();
        sendToCalc('E', "CLAUDE SENT AN EMPTY REPLY. TRY AGAIN.");
        return;
    }

    addToHistory("assistant", reply);
    sendToCalc('R', toCalcText(reply));
}

void setup()
{
    Serial.begin(BAUD_RATE);
    history.to<JsonArray>();

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop()
{
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            inputLine.trim();
            if (inputLine.length() > 0) {
                askClaude(inputLine);
            }
            inputLine = "";
        } else if (c != '\r' && inputLine.length() < MAX_INPUT) {
            inputLine += c;
        }
    }
}
