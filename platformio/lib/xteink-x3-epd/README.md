Vendored subset of the FreeInk SDK (MIT License, see LICENSE):
https://github.com/Free-Ink/freeink-sdk

EpdBus, PanelDriver, the Xteink X3 panel drivers (UC8253 and UC8279d
variants, runtime-detected via XteinkDetect), their LUTs, and the
BoardConfig profile header. Used by the BOARD_XTEINK_X3 build
(xteink_x3 environment); see src/X3Display.h for the GxEPD2-style
facade that adapts them to this project.
