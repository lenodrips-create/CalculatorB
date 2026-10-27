#ifndef CAMERA_H
#define CAMERA_H

// 8-bit camera viewfinder. The calculator has no camera, so this is the
// look only: static, a scan line, REC light and timer.
//   [enter]  snap (flash + photo count)
//   [clear]  back to the welcome screen
void camera_screen(void);

#endif
