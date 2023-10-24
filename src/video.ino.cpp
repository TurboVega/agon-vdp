# 1 "C:\\Users\\restd\\AppData\\Local\\Temp\\tmp_8491g93"
#include <Arduino.h>
# 1 "D:/AgonLight2/agon-vdp/src/video.ino"
# 43 "D:/AgonLight2/agon-vdp/src/video.ino"
#include "fabgl.h"
#include "HardwareSerial.h"
#include "ESP32Time.h"
#include "src/di_manager.h"

#define VERSION 1
#define REVISION 4
#define RC 1

#define DEBUG 1
#define SERIALKB 0

fabgl::PS2Controller PS2Controller;
fabgl::Canvas * Canvas;
fabgl::SoundGenerator SoundGenerator;

fabgl::VGA2Controller VGAController2;
fabgl::VGA4Controller VGAController4;
fabgl::VGA8Controller VGAController8;
fabgl::VGA16Controller VGAController16;
fabgl::VGAController VGAController64;

fabgl::VGABaseController * VGAController;

fabgl::Terminal Terminal;

fabgl::PaintOptions gpo;
fabgl::PaintOptions tpo;

#include "agon.h"
#include "agon_fonts.h"
#include "agon_audio.h"
#include "agon_palette.h"

int VGAColourDepth;
Point textCursor;
Point * activeCursor;
Rect * activeViewport;
Point origin;
Point p1, p2, p3;
RGB888 gfg, gbg;
RGB888 tfg, tbg;
Rect defaultViewport;
Rect textViewport;
Rect graphicsViewport;
int fontW;
int fontH;
int canvasW;
int canvasH;
bool cursorEnabled = true;
byte cursorBehaviour = 0;
bool useViewports = false;
bool logicalCoords = true;
double logicalScaleX;
double logicalScaleY;
int count = 0;
uint8_t numsprites = 0;
uint8_t current_sprite = 0;
uint8_t current_bitmap = 0;
Bitmap bitmaps[MAX_BITMAPS];
Sprite sprites[MAX_SPRITES];
byte keycode = 0;
byte modifiers = 0;
bool terminalMode = false;
int videoMode;
bool pagedMode = false;
int pagedModeCount = 0;
int kbRepeatDelay = 500;
int kbRepeatRate = 100;
bool initialised = false;
bool doWaitCompletion;
uint8_t palette[64];

audio_channel * audio_channels[AUDIO_CHANNELS];

ESP32Time rtc(0);

DiManager* di_manager;

#if DEBUG == 1 || SERIALKB == 1
HardwareSerial DBGSerial(0);
#endif

#include "src/di_bitmap.h"
#define __root 
#define _COMPILE_HEX_DATA_ 
#include "00187SCx128X4.h"

uint32_t the_offset = 0;
void IRAM_ATTR on_vertical_blank_start();
void IRAM_ATTR on_lines_painted();
void otf(void * pvParameters);
void setup();
void loop();
void boot_screen();
int readByte_t();
int readWord_t();
byte readByte_b();
uint32_t readLong_b();
void copy_font();
void setRTSStatus(bool value);
void init_audio();
void init_audio_channel(int channel);
void audio_driver(void * parameters);
void wait_eZ80();
void sendCursorPosition();
void sendScreenChar(int x, int y);
void sendScreenPixel(int x, int y);
void sendPlayNote(int channel, int success);
void sendModeInformation();
void sendTime();
void sendKeyboardState();
void sendGeneralPoll();
void cls(bool resetViewports);
void clg();
void clearViewport(Rect * viewport);
char get_screen_char(int px, int py);
bool cmp_char(uint8_t * c1, uint8_t *c2, int len);
void switchTerminalMode();
void setPaletteItem(int l, RGB888 c);
void updateRGB2PaletteLUT();
void resetPalette(const uint8_t colours[]);
int change_resolution(int colours, char * modeLine);
int change_mode(int mode);
void set_mode(int mode);
void print(char const * text);
void printFmt(const char *format, ...);
void do_keyboard_terminal();
void wait_shiftkey();
void do_keyboard();
void send_packet(byte code, byte len, byte data[]);
void do_cursor();
Point scale(Point p);
Point scale(int X, int Y);
Point translateViewport(Point p);
Point translateViewport(int X, int Y);
Point translateCanvas(Point p);
Point translateCanvas(int X, int Y);
void vdu(byte c);
void cursorLeft();
void cursorRight();
void cursorDown();
void cursorUp();
void cursorCR();
void cursorHome();
void cursorTab();
void vdu_mode();
void vdu_graphicsViewport();
void vdu_resetViewports();
void vdu_textViewport();
void vdu_origin();
void vdu_colour();
void vdu_gcol();
void vdu_palette();
void vdu_plot();
void vdu_plot_triangle(byte mode);
void vdu_plot_circle(byte mode);
void vdu_sys();
void vdu_sys_udg(byte c);
void vdu_sys_video();
void vdu_sys_audio();
void vdu_sys_keystate();
void vdu_sys_video_kblayout(byte region);
void vdu_sys_video_time();
void vdu_sys_scroll();
void vdu_sys_cursorBehaviour();
word play_note(byte channel, byte volume, word frequency, word duration);
void vdu_sys_sprites(void);
#line 134 "D:/AgonLight2/agon-vdp/src/video.ino"
void IRAM_ATTR on_vertical_blank_start() {





do_keyboard();
}


void IRAM_ATTR on_lines_painted() {
}

void debug_log(const char *format, ...);

void otf(void * pvParameters) {
 debug_log("OTF task running\r\n");
 videoMode = 19;
 di_manager = new DiManager();
 di_manager->create_root();

 di_manager->create_solid_rectangle(40, ROOT_PRIMITIVE_ID, 1, 0, 0, 800, 600, 0x15|PIXEL_ALPHA_100_MASK);
# 179 "D:/AgonLight2/agon-vdp/src/video.ino"
 di_manager->set_on_vertical_blank_cb(&on_vertical_blank_start);
# 196 "D:/AgonLight2/agon-vdp/src/video.ino"
#define DO_RECTANGLES 0
#if DO_RECTANGLES
    uint32_t id = 20;
 for (uint16_t b = 0; b < 2; b++) {
  for (uint16_t g = 0; g < 2; g++) {
   for (uint16_t r = 0; r < 2; r++) {
    uint8_t c = PIXEL_COLOR_ABGR(PIXEL_ALPHA_100, (b*3), (g*3), (r*3));
    uint16_t x = (b*4+g*2+r)*100;
    di_manager->create_solid_rectangle(id++, ROOT_PRIMITIVE_ID, 1, x, 0, 100, 600, c);
   }
  }
 }

    uint16_t y = 2;
 for (uint16_t b = 0; b < 2; b++) {
  for (uint16_t g = 0; g < 2; g++) {
   for (uint16_t r = 0; r < 2; r++) {
    for (uint16_t a = 0; a < 4; a++) {
     uint8_t c = PIXEL_COLOR_ABGR(a, (b*3), (g*3), (r*3));
     di_manager->create_solid_rectangle(id++, ROOT_PRIMITIVE_ID, 1, 50, y, 700, 16, c);
     y += 16+2;
    }
   }
  }
 }
#endif

#define DO_ELLIPSES 0
#if DO_ELLIPSES
    double twopi = PI*2.0;
 for (int c = 0; c<64; c++) {
  double a1 = twopi * c / 64.0;
  double a2 = twopi * (c + 1) / 64.0;

  double cos1 = cos(a1);
  double cos2 = cos(a2);
  double sin1 = sin(a1);
  double sin2 = sin(a2);

  double w1 = 80.0;
  double w2 = 100.0;
  double h1 = 130.0;
  double h2 = 150.0;

  int32_t x1 = 400 + (int32_t)(w1 * cos1);
  int32_t y1 = 300 + (int32_t)(h1 * sin1);
  int32_t x2 = 400 + (int32_t)(w2 * cos1);
  int32_t y2 = 300 + (int32_t)(h2 * sin1);

  int32_t x3 = 400 + (int32_t)(w1 * cos2);
  int32_t y3 = 300 + (int32_t)(h1 * sin2);
  int32_t x4 = 400 + (int32_t)(w2 * cos2);
  int32_t y4 = 300 + (int32_t)(h2 * sin2);

  di_manager->create_triangle(150+c*2, ROOT_PRIMITIVE_ID, 1, x1, y1, x2, y2, x4, y4, c|PIXEL_ALPHA_100_MASK);
  di_manager->create_triangle(151+c*2, ROOT_PRIMITIVE_ID, 1, x3, y3, x1, y1, x4, y4, c|PIXEL_ALPHA_100_MASK);
# 270 "D:/AgonLight2/agon-vdp/src/video.ino"
 }
#endif






#define DRAW_SEVERAL_BITMAPS 1
#if DRAW_SEVERAL_BITMAPS
 #define BM_WIDTH 128
 #define BM_HEIGHT 90

 auto prim0 = di_manager->create_transparent_bitmap(100, ROOT_PRIMITIVE_ID, PRIM_FLAG_PAINT_THIS|PRIM_FLAGS_BLENDED, BM_WIDTH, BM_HEIGHT, 0xC0);
 auto prim1 = di_manager->create_transparent_bitmap(101, ROOT_PRIMITIVE_ID, PRIM_FLAG_PAINT_THIS, BM_WIDTH, BM_HEIGHT, 0xC0);
# 293 "D:/AgonLight2/agon-vdp/src/video.ino"
 int i = 0;
 for (int y = 0; y < BM_HEIGHT; y++) {
  for (int x = 0; x < BM_WIDTH; x++) {
   uint8_t c = ((g_00187SCx128X4Data[i]>>6)<<4) | ((g_00187SCx128X4Data[i+1]>>6)<<2) | ((g_00187SCx128X4Data[i+2]>>6));
   i += 3;




   if (x == 17 || x>=33 && x<=41 || y==27 || y==40 || x==125) c=0x00;
   prim0->set_transparent_pixel(x, y, c|PIXEL_ALPHA_100_MASK);
   prim1->set_transparent_pixel(x, y, c|PIXEL_ALPHA_100_MASK);


  }

 }

 di_manager->move_primitive_absolute(100, 100, 100);
 di_manager->move_primitive_absolute(101, 100, 190);







#endif

#define DRAW_SEVERAL_RENDERS 0
#if DRAW_SEVERAL_RENDERS
 #define BM_WIDTH 160
 #define BM_HEIGHT 120

 auto prim0 = di_manager->create_solid_render(100, ROOT_PRIMITIVE_ID, PRIM_FLAG_PAINT_THIS, BM_WIDTH, BM_HEIGHT);
 auto prim1 = di_manager->create_solid_render(101, ROOT_PRIMITIVE_ID, PRIM_FLAG_PAINT_THIS, BM_WIDTH, BM_HEIGHT);
 auto prim2 = di_manager->create_solid_render(102, ROOT_PRIMITIVE_ID, PRIM_FLAG_PAINT_THIS, BM_WIDTH, BM_HEIGHT);
 auto prim3 = di_manager->create_solid_render(103, ROOT_PRIMITIVE_ID, PRIM_FLAG_PAINT_THIS, BM_WIDTH, BM_HEIGHT);

    prim0->render();
    prim1->render();
    prim2->render();
    prim3->render();

 di_manager->move_primitive_absolute(100, 100, 120);
 di_manager->move_primitive_absolute(101, 500, 130);
 di_manager->move_primitive_absolute(102, 300, 20);
 di_manager->move_primitive_absolute(103, 340, 360);
#endif

#define DRAW_TILE_MAP 0
#if DRAW_TILE_MAP
#define TM_ROWS 6
#define TM_COLS 6
    DiTileMap* tile_map = di_manager->create_tile_map(500, ROOT_PRIMITIVE_ID, PRIM_FLAGS_DEFAULT|PRIM_FLAGS_ALL_SAME,
                            ACT_PIXELS, ACT_LINES, TM_COLS, TM_ROWS, BM_WIDTH, BM_HEIGHT);
 tile_map->create_bitmap(1);
 int i = 0;
 for (int y = 0; y < BM_HEIGHT; y++) {
  for (int x = 0; x < BM_WIDTH; x++) {
   uint8_t c = ((g_00187SCx128X4Data[i]>>6)<<4) | ((g_00187SCx128X4Data[i+1]>>6)<<2) | ((g_00187SCx128X4Data[i+2]>>6));
   i += 3;
   tile_map->set_pixel(1, x, y, c|PIXEL_ALPHA_100_MASK);
  }
 }

 for (uint16_t row = 0; row < TM_ROWS; row++) {
  for (uint16_t col = 0; col < TM_COLS; col++) {
   tile_map->set_tile(col, row, 1);
  }
 }
 di_manager->move_primitive_relative(500, 0, 0);
#endif

#define DRAW_TILE_ARRAY 0
#if DRAW_TILE_ARRAY
#define TM_ROWS 6
#define TM_COLS 6
    DiTileArray* tile_array = di_manager->create_tile_array(500, ROOT_PRIMITIVE_ID, PRIM_FLAGS_DEFAULT|PRIM_FLAGS_ALL_SAME,
                            ACT_PIXELS, ACT_LINES, TM_COLS, TM_ROWS, BM_WIDTH, BM_HEIGHT);
 tile_array->create_bitmap(1);
 int i = 0;
 for (int y = 0; y < BM_HEIGHT; y++) {
  for (int x = 0; x < BM_WIDTH; x++) {
   uint8_t c = ((g_00187SCx128X4Data[i]>>6)<<4) | ((g_00187SCx128X4Data[i+1]>>6)<<2) | ((g_00187SCx128X4Data[i+2]>>6));
   i += 3;
   tile_array->set_pixel(1, x, y, c|PIXEL_ALPHA_100_MASK);
  }
 }

 for (uint16_t row = 0; row < TM_ROWS; row++) {
  for (uint16_t col = 0; col < TM_COLS; col++) {
   tile_array->set_tile(col, row, 1);
  }
 }
 di_manager->move_primitive_relative(500, 0, 0);
#endif

 debug_log("Running OTF manager...\r\n");
 delay(3000);
 di_manager->run();
}

void setup() {
 disableCore0WDT(); delay(200);
 disableCore1WDT(); delay(200);
 #if DEBUG == 1 || SERIALKB == 1
 DBGSerial.begin(500000, SERIAL_8N1, 3, 1);
 #endif
 ESPSerial.end();
  ESPSerial.setRxBufferSize(UART_RX_SIZE);
  ESPSerial.begin(UART_BR, SERIAL_8N1, UART_RX, UART_TX);
 #if USE_HWFLOW == 1
 ESPSerial.setHwFlowCtrlMode(HW_FLOWCTRL_RTS, 64);
 ESPSerial.setPins(UART_NA, UART_NA, UART_CTS, UART_RTS);
 #else
 pinMode(UART_RTS, OUTPUT);
 pinMode(UART_CTS, INPUT);
 setRTSStatus(true);
 #endif
 wait_eZ80();
  PS2Controller.begin(PS2Preset::KeyboardPort0, KbdMode::CreateVirtualKeysQueue);
 PS2Controller.keyboard()->setLayout(&fabgl::UKLayout);
 PS2Controller.keyboard()->setCodePage(fabgl::CodePages::get(1252));
 PS2Controller.keyboard()->setTypematicRateAndDelay(kbRepeatRate, kbRepeatDelay);

 copy_font();



 TaskHandle_t xHandle = NULL;
 xTaskCreatePinnedToCore(otf, "OTF-MODE", 4096, NULL,
       OTF_MANAGER_PRIORITY, &xHandle, 1);
}



void loop() {
 if (di_manager) {
  return;
 }

 bool cursorVisible = false;
 bool cursorState = false;

 while(true) {
  if(terminalMode) {
   do_keyboard_terminal();
   continue;
  }
     cursorVisible = ((count & 0xFFFF) == 0);
     if(cursorVisible) {
        cursorState = !cursorState;
        do_cursor();
     }
     do_keyboard();
     if(ESPSerial.available() > 0) {
   #if USE_HWFLOW == 0
   if(ESPSerial.available() > UART_RX_THRESH) {
    setRTSStatus(false);
   }
   #endif
        if(cursorState) {
        cursorState = false;
          do_cursor();
        }
        byte c = ESPSerial.read();
        vdu(c);
     }
  #if USE_HWFLOW == 0
  else {
   setRTSStatus(true);
  }
  #endif
     count++;
   }
}



void boot_screen() {
   printFmt("Agon Quark VDP Version %d.%02d", VERSION, REVISION);
 #if RC > 0
    printFmt(" RC%d", RC);
 #endif
 printFmt("\n\r");
}



void debug_log(const char *format, ...) {
 #if DEBUG == 1
    va_list ap;
    va_start(ap, format);
    int size = vsnprintf(nullptr, 0, format, ap) + 1;
    if (size > 0) {
     va_end(ap);
      va_start(ap, format);
      char buf[size + 1];
      vsnprintf(buf, size, format, ap);
      DBGSerial.print(buf);
    }
    va_end(ap);
 #endif
}





int readByte_t() {
 int i;
 unsigned long t = millis();

 while(millis() - t < 1000) {
  if(ESPSerial.available() > 0) {
   return ESPSerial.read();
  }
 }
 return -1;
}





int readWord_t() {
 int l = readByte_t();
 if(l >= 0) {
  int h = readByte_t();
  if(h >= 0) {
   return (h << 8) | l;
  }
 }
 return -1;
}



byte readByte_b() {
   while(ESPSerial.available() == 0);
   return ESPSerial.read();
}



uint32_t readLong_b() {
  uint32_t temp;
  temp = readByte_b();
  temp |= (readByte_b() << 8);
  temp |= (readByte_b() << 16);
  temp |= (readByte_b() << 24);
  return temp;
}



void copy_font() {
 memcpy(fabgl::FONT_AGON_DATA + 256, fabgl::FONT_AGON_BITMAP, sizeof(fabgl::FONT_AGON_BITMAP));
}



void setRTSStatus(bool value) {
 digitalWrite(UART_RTS, value ? LOW : HIGH);
}



void init_audio() {
 for(int i = 0; i < AUDIO_CHANNELS; i++) {
  init_audio_channel(i);
 }
}
void init_audio_channel(int channel) {
   xTaskCreatePinnedToCore(audio_driver, "audio_driver",
     4096,
        &channel,
        PLAY_SOUND_PRIORITY,
        NULL,
     0
 );
}



void audio_driver(void * parameters) {
 int channel = *(int *)parameters;

 audio_channels[channel] = new audio_channel(channel);
 while(true) {
  audio_channels[channel]->loop();
  vTaskDelay(1);
 }
}



void wait_eZ80() {
 debug_log("wait_eZ80: Start\n\r");
 while(!initialised) {
     if(ESPSerial.available() > 0) {
   #if USE_HWFLOW == 0
   if(ESPSerial.available() > UART_RX_THRESH) {
    setRTSStatus(false);
   }
   #endif
   byte c = ESPSerial.read();
   if(c == 23) {
    vdu_sys();
   }
  }
 }
 debug_log("wait_eZ80: End\n\r");
}



void sendCursorPosition() {
 byte packet[] = {
  textCursor.X / fontW,
  textCursor.Y / fontH,
 };
 send_packet(PACKET_CURSOR, sizeof packet, packet);
}



void sendScreenChar(int x, int y) {
 int px = x * fontW;
 int py = y * fontH;
 char c = get_screen_char(px, py);
 byte packet[] = {
  c,
 };
 send_packet(PACKET_SCRCHAR, sizeof packet, packet);
}



void sendScreenPixel(int x, int y) {
 RGB888 pixel;
 byte pixelIndex = 0;
 Point p = translateViewport(scale(x, y));



 if(p.X >= 0 && p.Y >= 0 && p.X < canvasW && p.Y < canvasH) {
  pixel = Canvas->getPixel(p.X, p.Y);
  for(byte i = 0; i < VGAColourDepth; i++) {
   if(colourLookup[palette[i]] == pixel) {
    pixelIndex = i;
    break;
   }
  }
 }
 byte packet[] = {
  pixel.R,
  pixel.G,
  pixel.B,
  pixelIndex,
 };
 send_packet(PACKET_SCRPIXEL, sizeof packet, packet);
}



void sendPlayNote(int channel, int success) {
 byte packet[] = {
  channel,
  success,
 };
 send_packet(PACKET_AUDIO, sizeof packet, packet);
}



void sendModeInformation() {
 byte packet[] = {
  canvasW & 0xFF,
  (canvasW >> 8) & 0xFF,
  canvasH & 0xFF,
  (canvasH >> 8) & 0xFF,
  canvasW / fontW,
  canvasH / fontH,
  VGAColourDepth,
  videoMode,
 };
 send_packet(PACKET_MODE, sizeof packet, packet);
}



void sendTime() {
 byte packet[] = {
  rtc.getYear() - EPOCH_YEAR,
  rtc.getMonth(),
  rtc.getDay(),
  rtc.getDayofYear(),
  rtc.getDayofWeek(),
  rtc.getHour(true),
  rtc.getMinute(),
  rtc.getSecond(),
 };
 send_packet(PACKET_RTC, sizeof packet, packet);
}



void sendKeyboardState() {
 bool numLock;
 bool capsLock;
 bool scrollLock;
 PS2Controller.keyboard()->getLEDs(&numLock, &capsLock, &scrollLock);
 byte packet[] = {
  kbRepeatDelay & 0xFF,
  (kbRepeatDelay >> 8) & 0xFF,
  kbRepeatRate & 0xFF,
  (kbRepeatRate >> 8) & 0xFF,
  scrollLock | (capsLock << 1) | (numLock << 2)
 };
 send_packet(PACKET_KEYSTATE, sizeof packet, packet);
}



void sendGeneralPoll() {
 byte b = readByte_b();
 byte packet[] = {
  b,
 };
 send_packet(PACKET_GP, sizeof packet, packet);
 initialised = true;
}



void cls(bool resetViewports) {
 int i;

 if(resetViewports) {
  vdu_resetViewports();
 }
 if(Canvas) {
  Canvas->setPenColor(tfg);
   Canvas->setBrushColor(tbg);
  Canvas->setPaintOptions(tpo);
  clearViewport(&textViewport);
 }
 if(numsprites) {
  if(VGAController) {
   VGAController->removeSprites();
   clearViewport(&textViewport);
  }
  numsprites = 0;
 }
 textCursor = Point(activeViewport->X1, activeViewport->Y1);
 pagedModeCount = 0;
}



void clg() {
 if(Canvas) {
  Canvas->setPenColor(gfg);
   Canvas->setBrushColor(gbg);
  Canvas->setPaintOptions(gpo);
  clearViewport(&graphicsViewport);
 }
}



void clearViewport(Rect * viewport) {
 if(Canvas) {
  if(useViewports) {
   Canvas->fillRectangle(*viewport);
  }
  else {
   Canvas->clear();
  }
 }
}



fabgl::PaintOptions getPaintOptions(int mode, fabgl::PaintOptions priorPaintOptions) {
 fabgl::PaintOptions p = priorPaintOptions;

 switch(mode) {
  case 0: p.NOT = 0; p.swapFGBG = 0; break;
  case 4: p.NOT = 1; p.swapFGBG = 0; break;
 }
 return p;
}



char get_screen_char(int px, int py) {
 RGB888 pixel;
 uint8_t charRow;
 uint8_t charData[8];
 uint8_t R = tbg.R;
 uint8_t G = tbg.G;
 uint8_t B = tbg.B;



 if(px < 0 || py < 0 || px >= canvasW - 8 || py >= canvasH - 8) {
  return 0;
 }



 for(int y = 0; y < 8; y++) {
  charRow = 0;
  for(int x = 0; x < 8; x++) {
   pixel = Canvas->getPixel(px + x, py + y);
   if(!(pixel.R == R && pixel.G == G && pixel.B == B)) {
    charRow |= (0x80 >> x);
   }
  }
  charData[y] = charRow;
 }



 for(int i = 32; i <= 255; i++) {
  if(cmp_char(charData, &fabgl::FONT_AGON_DATA[i * 8], 8)) {
   return i;
  }
 }
 return 0;
}

bool cmp_char(uint8_t * c1, uint8_t *c2, int len) {
 for(int i = 0; i < len; i++) {
  if(*c1++ != *c2++) {
   return false;
  }
 }
 return true;
}



void switchTerminalMode() {
 cls(true);
   delete Canvas;
 Terminal.begin(VGAController);
 Terminal.connectSerialPort(ESPSerial);
 Terminal.enableCursor(true);
 terminalMode = true;
}







fabgl::VGABaseController * get_VGAController(int colours) {
 switch(colours) {
  case 2: return VGAController2.instance();
  case 4: return VGAController4.instance();
  case 8: return VGAController8.instance();
  case 16: return VGAController16.instance();
  case 64: return VGAController64.instance();
 }
 return nullptr;
}






void setPaletteItem(int l, RGB888 c) {
 if(l < VGAColourDepth) {
  switch(VGAColourDepth) {
   case 2: VGAController2.setPaletteItem(l, c); break;
   case 4: VGAController4.setPaletteItem(l, c); break;
   case 8: VGAController8.setPaletteItem(l, c); break;
   case 16: VGAController16.setPaletteItem(l, c); break;
  }
 }
}



void updateRGB2PaletteLUT() {
 switch(VGAColourDepth) {
  case 2: VGAController2.updateRGB2PaletteLUT(); break;
  case 4: VGAController4.updateRGB2PaletteLUT(); break;
  case 8: VGAController8.updateRGB2PaletteLUT(); break;
  case 16: VGAController16.updateRGB2PaletteLUT(); break;
 }
}






void resetPalette(const uint8_t colours[]) {
 for(int i = 0; i < 64; i++) {
  uint8_t c = colours[i%VGAColourDepth];
  palette[i] = c;
  setPaletteItem(i, colourLookup[c]);
 }
 updateRGB2PaletteLUT();
}
# 915 "D:/AgonLight2/agon-vdp/src/video.ino"
int change_resolution(int colours, char * modeLine) {
 fabgl::VGABaseController * controller = get_VGAController(colours);

 if(controller == nullptr) {
  return 1;
 }
   delete Canvas;

 VGAColourDepth = colours;
 if(VGAController != controller) {
  if(VGAController) {
   VGAController->end();
  }
  VGAController = controller;
  VGAController->begin();
 }
 if(modeLine) {
  VGAController->setResolution(modeLine);
 }
 VGAController->enableBackgroundPrimitiveExecution(true);
 VGAController->enableBackgroundPrimitiveTimeout(false);

   Canvas = new fabgl::Canvas(VGAController);



 if(VGAController->getScreenHeight() != VGAController->getViewPortHeight()) {
  return 2;
 }
 return 0;
}
# 956 "D:/AgonLight2/agon-vdp/src/video.ino"
int change_mode(int mode) {
 int errVal = -1;

 cls(true);
 if(mode != videoMode) {
  switch(mode) {
   case 0:
    errVal = change_resolution(2, SVGA_1024x768_60Hz);
    break;
   case 1:
    errVal = change_resolution(16, VGA_512x384_60Hz);
    break;
   case 2:
    errVal = change_resolution(64, QVGA_320x240_60Hz);
    break;
   case 3:
    errVal = change_resolution(16, VGA_640x480_60Hz);
    break;
  }
  if(errVal != 0) {
   return errVal;
  }
 }
 switch(VGAColourDepth) {
  case 2: resetPalette(defaultPalette02); break;
  case 4: resetPalette(defaultPalette04); break;
  case 8: resetPalette(defaultPalette08); break;
  case 16: resetPalette(defaultPalette10); break;
  case 64: resetPalette(defaultPalette40); break;
 }
 tpo = getPaintOptions(0, tpo);
 gpo = getPaintOptions(0, gpo);
  gfg = colourLookup[0x3F];
 gbg = colourLookup[0x00];
 tfg = colourLookup[0x3F];
 tbg = colourLookup[0x00];
   Canvas->selectFont(&fabgl::FONT_AGON);
   Canvas->setGlyphOptions(GlyphOptions().FillBackground(true));
   Canvas->setPenWidth(1);
 origin = Point(0,0);
 p1 = Point(0,0);
 p2 = Point(0,0);
 p3 = Point(0,0);
 canvasW = Canvas->getWidth();
 canvasH = Canvas->getHeight();
 fontW = Canvas->getFontInfo()->width;
 fontH = Canvas->getFontInfo()->height;
 logicalScaleX = LOGICAL_SCRW / (double)canvasW;
 logicalScaleY = LOGICAL_SCRH / (double)canvasH;
 cursorEnabled = true;
 cursorBehaviour = 0;
 activeCursor = &textCursor;
 vdu_resetViewports();
 sendModeInformation();
 debug_log("do_modeChange: canvas(%d,%d), scale(%f,%f)\n\r", canvasW, canvasH, logicalScaleX, logicalScaleY);
 return 0;
}






void set_mode(int mode) {
 int errVal = change_mode(mode);
 if(errVal != 0) {
  debug_log("set_mode: error %d\n\r", errVal);
  change_mode(videoMode);
  return;
 }
 videoMode = mode;
}

void print(char const * text) {
 if (di_manager) {
  di_manager->store_string((const uint8_t*) text);
 } else {
  for(int i = 0; i < strlen(text); i++) {
   vdu(text[i]);
  }
 }
}

void printFmt(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int size = vsnprintf(nullptr, 0, format, ap) + 1;
    if (size > 0) {
     va_end(ap);
      va_start(ap, format);
      char buf[size + 1];
      vsnprintf(buf, size, format, ap);
      print(buf);
    }
    va_end(ap);
 }



void do_keyboard_terminal() {
   fabgl::Keyboard* kb = PS2Controller.keyboard();
 fabgl::VirtualKeyItem item;



 if(kb->getNextVirtualKey(&item, 0)) {
  if(item.down) {
   ESPSerial.write(item.ASCII);
  }
 }



 while(ESPSerial.available()) {
  Terminal.write(ESPSerial.read());
 }
}



void wait_shiftkey() {
   fabgl::Keyboard* kb = PS2Controller.keyboard();
 fabgl::VirtualKeyItem item;



 do {
  kb->getNextVirtualKey(&item, 0);
 } while(item.SHIFT);



 do {
  kb->getNextVirtualKey(&item, 0);
  if(item.ASCII == 27) {
   byte packet[] = {
    item.ASCII,
    0,
    item.vk,
    item.down,
   };
   send_packet(PACKET_KEYCODE, sizeof packet, packet);
   return;
  }
 } while(!item.SHIFT);
}



void do_keyboard() {
   fabgl::Keyboard* kb = PS2Controller.keyboard();
 fabgl::VirtualKeyItem item;

 #if SERIALKB == 1
 if(DBGSerial.available()) {
  byte packet[] = {
   DBGSerial.read(),
   0,
  };
  send_packet(PACKET_KEYCODE, sizeof packet, packet);
  return;
 }
 #endif

 if(kb->getNextVirtualKey(&item, 0)) {
  if(item.down) {
   switch(item.vk) {
    case fabgl::VK_LEFT:
     keycode = 0x08;
     break;
    case fabgl::VK_TAB:
     keycode = 0x09;
     break;
    case fabgl::VK_RIGHT:
     keycode = 0x15;
     break;
    case fabgl::VK_DOWN:
     keycode = 0x0A;
     break;
    case fabgl::VK_UP:
     keycode = 0x0B;
     break;
    case fabgl::VK_BACKSPACE:
     keycode = 0x7F;
     break;
    default:
     keycode = item.ASCII;
     break;
   }


   modifiers =
    item.CTRL << 0 |
    item.SHIFT << 1 |
    item.LALT << 2 |
    item.RALT << 3 |
    item.CAPSLOCK << 4 |
    item.NUMLOCK << 5 |
    item.SCROLLLOCK << 6 |
    item.GUI << 7
   ;
  }


  switch(keycode) {
   case 14: pagedMode = true; break;
   case 15: pagedMode = false; break;
  }


  byte packet[] = {
   keycode,
   modifiers,
   item.vk,
   item.down,
  };
  send_packet(PACKET_KEYCODE, sizeof packet, packet);
 }
}



void send_packet(byte code, byte len, byte data[]) {
 ESPSerial.write(code + 0x80);
 ESPSerial.write(len);
 for(int i = 0; i < len; i++) {
  ESPSerial.write(data[i]);
 }
}



void do_cursor() {
 if(cursorEnabled) {
    Canvas->swapRectangle(textCursor.X, textCursor.Y, textCursor.X + fontW - 1, textCursor.Y + fontH - 1);
 }
}



Point scale(Point p) {
 return scale(p.X, p.Y);
}
Point scale(int X, int Y) {
 if(logicalCoords) {
  return Point((double)X / logicalScaleX, (double)Y / logicalScaleY);
 }
 return Point(X, Y);
}



Point translateViewport(Point p) {
 return translateViewport(p.X, p.Y);
}
Point translateViewport(int X, int Y) {
 if(logicalCoords) {
  return Point(graphicsViewport.X1 + (origin.X + X), graphicsViewport.Y2 - (origin.Y + Y));
 }
 return Point(graphicsViewport.X1 + (origin.X + X), graphicsViewport.Y1 + (origin.Y + Y));
}



Point translateCanvas(Point p) {
 return translateCanvas(p.X, p.Y);
}
Point translateCanvas(int X, int Y) {
 if(logicalCoords) {
  return Point(origin.X + X, (canvasH - 1) - (origin.Y + Y));
 }
 return Point(origin.X + X, origin.Y + Y);
}

void vdu(byte c) {
 bool useTextCursor = (activeCursor == &textCursor);

 if(c >= 0x20 && c != 0x7F) {
  if(useTextCursor) {
   Canvas->setClippingRect(defaultViewport);
   Canvas->setPenColor(tfg);
   Canvas->setBrushColor(tbg);
   Canvas->setPaintOptions(tpo);
  }
  else {
   Canvas->setClippingRect(graphicsViewport);
   Canvas->setPenColor(gfg);
   Canvas->setPaintOptions(gpo);
  }
  Canvas->drawChar(activeCursor->X, activeCursor->Y, c);
  cursorRight();
 }
 else {
  doWaitCompletion = false;
  switch(c) {
   case 0x04:
      Canvas->setGlyphOptions(GlyphOptions().FillBackground(true));
    activeCursor = &textCursor;
    activeViewport = &textViewport;
    break;
   case 0x05:
      Canvas->setGlyphOptions(GlyphOptions().FillBackground(false));
    activeCursor = &p1;
    activeViewport = &graphicsViewport;
    break;
   case 0x07:
    play_note(0, 100, 750, 125);
    break;
   case 0x08:
    cursorLeft();
    break;
   case 0x09:
    cursorRight();
    break;
   case 0x0A:
    cursorDown();
    break;
   case 0x0B:
    cursorUp();
    break;
   case 0x0C:
    cls(false);
    break;
   case 0x0D:
    cursorCR();
    break;
   case 0x0E:
    pagedMode = true;
    break;
   case 0x0F:
    pagedMode = false;
    break;
   case 0x10:
    clg();
    break;
   case 0x11:
    vdu_colour();
    break;
   case 0x12:
    vdu_gcol();
    break;
   case 0x13:
    vdu_palette();
    break;
   case 0x16:
    vdu_mode();
    break;
   case 0x17:
    vdu_sys();
    break;
   case 0x18:
    vdu_graphicsViewport();
    break;
   case 0x19:
    vdu_plot();
    break;
   case 0x1A:
    vdu_resetViewports();
    break;
   case 0x1C:
    vdu_textViewport();
    break;
   case 0x1D:
    vdu_origin();
   case 0x1E:
    cursorHome();
    break;
   case 0x1F:
    cursorTab();
    break;
   case 0x7F:
    cursorLeft();
    Canvas->setBrushColor(useTextCursor ? tbg : gbg);
    Canvas->fillRectangle(activeCursor->X, activeCursor->Y, activeCursor->X + fontW - 1, activeCursor->Y + fontH - 1);
    break;
  }
  if(doWaitCompletion) {
   Canvas->waitCompletion(false);
  }
 }
}



void cursorLeft() {
 activeCursor->X -= fontW;
   if(activeCursor->X < activeViewport->X1) {
     activeCursor->X = activeViewport->X1;
   }
}



void cursorRight() {
   activeCursor->X += fontW;
   if(activeCursor->X > activeViewport->X2) {
  if(activeCursor == &textCursor || (~cursorBehaviour & 0x40)) {
      cursorCR();
      cursorDown();
  }
   }
}



void cursorDown() {
 activeCursor->Y += fontH;



 if(activeCursor != &textCursor) {
  if(activeCursor->Y > activeViewport->Y2) {
   activeCursor->Y = activeViewport->Y2;
  }
  return;
 }



 if(pagedMode) {
  pagedModeCount++;
  if(pagedModeCount >= (activeViewport->Y2 - activeViewport->Y1 + 1) / fontH) {
   pagedModeCount = 0;
   wait_shiftkey();
  }
 }



 if(activeCursor->Y > activeViewport->Y2) {
  activeCursor->Y -= fontH;
  if(~cursorBehaviour & 0x01) {
   Canvas->setScrollingRegion(activeViewport->X1, activeViewport->Y1, activeViewport->X2, activeViewport->Y2);
   Canvas->scroll(0, -fontH);
  }
  else {
   activeCursor->X = activeViewport->X2 + 1;
  }
 }
}



void cursorUp() {
   activeCursor->Y -= fontH;
   if(activeCursor->Y < activeViewport->Y1) {
  activeCursor->Y = activeViewport->Y1;
   }
}



void cursorCR() {
   activeCursor->X = activeViewport->X1;
}



void cursorHome() {
 activeCursor->X = activeViewport->X1;
 activeCursor->Y = activeViewport->Y1;
}



void cursorTab() {
 int x = readByte_t();
 if(x >= 0) {
  int y = readByte_t();
  if(y >= 0) {
   activeCursor->X = x * fontW;
   activeCursor->Y = y * fontH;
  }
 }
}



void vdu_mode() {
   int mode = readByte_t();
 debug_log("vdu_mode: %d\n\r", mode);
 if(mode >= 0) {
    set_mode(mode);
 }
}




void vdu_graphicsViewport() {
 int x1 = readWord_t();
 int y2 = readWord_t();
 int x2 = readWord_t();
 int y1 = readWord_t();

 Point p1 = translateCanvas(scale((short)x1, (short)y1));
 Point p2 = translateCanvas(scale((short)x2, (short)y2));

 if(p1.X >= 0 && p2.X < canvasW && p1.Y >= 0 && p2.Y < canvasH && p2.X > p1.X && p2.Y > p1.Y) {
  graphicsViewport = Rect(p1.X, p1.Y, p2.X, p2.Y);
  useViewports = true;
  debug_log("vdu_graphicsViewport: OK %d,%d,%d,%d\n\r", p1.X, p1.Y, p2.X, p2.Y);
 }
 else {
  debug_log("vdu_graphicsViewport: Invalid Viewport %d,%d,%d,%d\n\r", p1.X, p1.Y, p2.X, p2.Y);
 }
 Canvas->setClippingRect(graphicsViewport);
}



void vdu_resetViewports() {
 defaultViewport = Rect(0, 0, canvasW - 1, canvasH - 1);
 textViewport = Rect(0, 0, canvasW - 1, canvasH - 1);
 graphicsViewport = Rect(0, 0, canvasW - 1, canvasH - 1);
 activeViewport = &textViewport;
 useViewports = false;
 debug_log("vdu_resetViewport\n\r");
}




void vdu_textViewport() {
 int x1 = readByte_t() * fontW;
 int y2 = (readByte_t() + 1) * fontH - 1;
 int x2 = (readByte_t() + 1) * fontW - 1;
 int y1 = readByte_t() * fontH;

 if(x2 >= canvasW) x2 = canvasW - 1;
 if(y2 >= canvasH) y2 = canvasH - 1;

 if(x1 >= 0 && y1 >= 0 && x2 > x1 && y2 > y1) {
  textViewport = Rect(x1, y1, x2, y2);
  useViewports = true;
  if(activeCursor->X < x1 || activeCursor->X > x2 || activeCursor->Y < y1 || activeCursor->Y > y2) {
   activeCursor->X = x1;
   activeCursor->Y = y1;
  }
  debug_log("vdu_textViewport: OK %d,%d,%d,%d\n\r", x1, y1, x2, y2);
 }
 else {
  debug_log("vdu_textViewport: Invalid Viewport %d,%d,%d,%d\n\r", x1, y1, x2, y2);
 }
}



void vdu_origin() {
 int x = readWord_t();
 if(x >= 0) {
  int y = readWord_t();
  if(y >= 0) {
   origin = scale(x, y);
   debug_log("vdu_origin: %d,%d\n\r", origin.X, origin.Y);
  }
 }
}



void vdu_colour() {
 int colour = readByte_t();
 byte c = palette[colour%VGAColourDepth];

 if(colour >= 0 && colour < 64) {
  tfg = colourLookup[c];
  debug_log("vdu_colour: tfg %d = %02X : %02X,%02X,%02X\n\r", colour, c, tfg.R, tfg.G, tfg.B);
 }
 else if(colour >= 128 && colour < 192) {
  tbg = colourLookup[c];
  debug_log("vdu_colour: tbg %d = %02X : %02X,%02X,%02X\n\r", colour, c, tbg.R, tbg.G, tbg.B);
 }
 else {
  debug_log("vdu_colour: invalid colour %d\n\r", colour);
 }
}



void vdu_gcol() {
 int mode = readByte_t();
 int colour = readByte_t();

 byte c = palette[colour%VGAColourDepth];

 if(mode >= 0 && mode <= 6) {
  if(colour >= 0 && colour < 64) {
   gfg = colourLookup[c];
   debug_log("vdu_gcol: mode %d, gfg %d = %02X : %02X,%02X,%02X\n\r", mode, colour, c, gfg.R, gfg.G, gfg.B);
  }
  else if(colour >= 128 && colour < 192) {
   gbg = colourLookup[c];
   debug_log("vdu_gcol: mode %d, gbg %d = %02X : %02X,%02X,%02X\n\r", mode, colour, c, gbg.R, gbg.G, gbg.B);
  }
  else {
   debug_log("vdu_gcol: invalid colour %d\n\r", colour);
  }
  gpo = getPaintOptions(mode, gpo);
 }
 else {
  debug_log("vdu_gcol: invalid mode %d\n\r", mode);
 }
}



void vdu_palette() {
 int l = readByte_t(); if(l == -1) return;
 int p = readByte_t(); if(p == -1) return;
 int r = readByte_t(); if(r == -1) return;
 int g = readByte_t(); if(g == -1) return;
 int b = readByte_t(); if(b == -1) return;

 RGB888 col;

 if(VGAColourDepth < 64) {
  if(p == 255) {
   col = RGB888(r, g, b);
  }
  else if(p < 64) {
   col = colourLookup[p];
  }
  else {
   debug_log("vdu_palette: p=%d not supported\n\r", p);
   return;
  }
  setPaletteItem(l, col);
  doWaitCompletion = true;
  debug_log("vdu_palette: %d,%d,%d,%d,%d\n\r", l, p, r, g, b);
 }
 else {
  debug_log("vdu_palette: not supported in this mode\n\r");
 }
}



void vdu_plot() {
   int mode = readByte_t(); if(mode == -1) return;

 int x = readWord_t(); if(x == -1) return; else x = (short)x;
 int y = readWord_t(); if(y == -1) return; else y = (short)y;

   p3 = p2;
   p2 = p1;
 p1 = translateViewport(scale(x, y));
 Canvas->setClippingRect(graphicsViewport);
 Canvas->setPenColor(gfg);
 Canvas->setPaintOptions(gpo);
 debug_log("vdu_plot: mode %d, (%d,%d) -> (%d,%d)\n\r", mode, x, y, p1.X, p1.Y);
   switch(mode) {
     case 0x04:
        Canvas->moveTo(p1.X, p1.Y);
        break;
     case 0x05:
        Canvas->lineTo(p1.X, p1.Y);
        break;
  case 0x40 ... 0x47:
   Canvas->setPixel(p1.X, p1.Y);
   break;
     case 0x50 ... 0x57:
        vdu_plot_triangle(mode);
        break;
     case 0x90 ... 0x97:
        vdu_plot_circle(mode);
        break;
   }
 doWaitCompletion = true;
}

void vdu_plot_triangle(byte mode) {
   Point p[3] = {
  p3,
  p2,
  p1,
 };
   Canvas->setBrushColor(gfg);
   Canvas->fillPath(p, 3);
   Canvas->setBrushColor(tbg);
}

void vdu_plot_circle(byte mode) {
   int a, b, r;
   switch(mode) {
     case 0x90 ... 0x93:
        r = 2 * (p1.X + p1.Y);
        Canvas->drawEllipse(p2.X, p2.Y, r, r);
        break;
     case 0x94 ... 0x97:
        a = p2.X - p1.X;
        b = p2.Y - p1.Y;
        r = 2 * sqrt(a * a + b * b);
        Canvas->drawEllipse(p2.X, p2.Y, r, r);
        break;
   }
}




void vdu_sys() {
   int mode = readByte_t();




 if(mode == -1) {
  return;
 }



 else if(mode < 32) {
    switch(mode) {
      case 0x00: {
         vdu_sys_video();
   } break;
   case 0x01: {
    int b = readByte_t();
    if(b >= 0) {
     cursorEnabled = b;
    }
   } break;
   case 0x07: {
    vdu_sys_scroll();
   } break;
   case 0x10: {
    vdu_sys_cursorBehaviour();
    break;
   }
   case 0x1B: {
    vdu_sys_sprites();
   } break;
    }
 }





 else {
  vdu_sys_udg(mode);
 }
}





void vdu_sys_udg(byte c) {
 uint8_t buffer[8];
 int b;

 for(int i = 0; i < 8; i++) {
  b = readByte_t();
  if(b == -1) {
   return;
  }
  buffer[i] = b;
 }
 memcpy(&fabgl::FONT_AGON_DATA[c * 8], buffer, 8);
}




void vdu_sys_video() {
   int mode = readByte_t();

   switch(mode) {
  case VDP_GP: {
   sendGeneralPoll();
  } break;
  case VDP_KEYCODE: {
   vdu_sys_video_kblayout(0);
  } break;
  case VDP_CURSOR: {
   sendCursorPosition();
  } break;
  case VDP_SCRCHAR: {
   int x = readWord_t();
   int y = readWord_t();
   sendScreenChar(x, y);
  } break;
  case VDP_SCRPIXEL: {
   int x = readWord_t();
   int y = readWord_t();
   sendScreenPixel((short)x, (short)y);
  } break;
  case VDP_AUDIO: {
   vdu_sys_audio();
  } break;
  case VDP_MODE: {
   sendModeInformation();
  } break;
  case VDP_RTC: {
   vdu_sys_video_time();
  } break;
  case VDP_KEYSTATE: {
   vdu_sys_keystate();
  } break;
  case VDP_LOGICALCOORDS: {
   int b = readByte_t();
   if(b >= 0) {
    logicalCoords = b;
   }
  } break;
  case VDP_TERMINALMODE: {
   switchTerminalMode();
  } break;
   }
}



void vdu_sys_audio() {
 int channel = readByte_t(); if(channel == -1) return;
 int waveform = readByte_t(); if(waveform == -1) return;
 int volume = readByte_t(); if(volume == -1) return;
 int frequency = readWord_t(); if(frequency == -1) return;
 int duration = readWord_t(); if(duration == -1) return;

 sendPlayNote(channel, play_note(channel, volume, frequency, duration));
}



void vdu_sys_keystate() {
 int d = readWord_t(); if(d == -1) return;
 int r = readWord_t(); if(r == -1) return;
 int b = readByte_t(); if(b == -1) return;

 if(d >= 250 && d <= 1000) kbRepeatDelay = (d / 250) * 250;
 if(r >= 33 && r <= 500) kbRepeatRate = r;

 if(b != 255) {
  PS2Controller.keyboard()->setLEDs(b & 4, b & 2, b & 1);
 }
 PS2Controller.keyboard()->setTypematicRateAndDelay(kbRepeatRate, kbRepeatDelay);
 debug_log("vdu_sys_video: keystate: d=%d, r=%d, led=%d\n\r", kbRepeatDelay, kbRepeatRate, b);
 sendKeyboardState();
}



void vdu_sys_video_kblayout(byte region) {
 if (!region) {
  region = readByte_t();
 }
 switch(region) {
  case 1: PS2Controller.keyboard()->setLayout(&fabgl::USLayout); break;
  case 2: PS2Controller.keyboard()->setLayout(&fabgl::GermanLayout); break;
  case 3: PS2Controller.keyboard()->setLayout(&fabgl::ItalianLayout); break;
  case 4: PS2Controller.keyboard()->setLayout(&fabgl::SpanishLayout); break;
  case 5: PS2Controller.keyboard()->setLayout(&fabgl::FrenchLayout); break;
  case 6: PS2Controller.keyboard()->setLayout(&fabgl::BelgianLayout); break;
  case 7: PS2Controller.keyboard()->setLayout(&fabgl::NorwegianLayout); break;
  case 8: PS2Controller.keyboard()->setLayout(&fabgl::JapaneseLayout);break;
  default:
   PS2Controller.keyboard()->setLayout(&fabgl::UKLayout);
   break;
 }
}



void vdu_sys_video_time() {
 int mode = readByte_t();

 if(mode == 0) {
  sendTime();
 }
 else if(mode == 1) {
  int yr = readByte_t(); if(yr == -1) return;
  int mo = readByte_t(); if(mo == -1) return;
  int da = readByte_t(); if(da == -1) return;
  int ho = readByte_t(); if(ho == -1) return;
  int mi = readByte_t(); if(mi == -1) return;
  int se = readByte_t(); if(se == -1) return;

  yr = EPOCH_YEAR + (int8_t)yr;

  if(yr >= 1970) {
   rtc.setTime(se, mi, ho, da, mo, yr);
  }
 }
}



void vdu_sys_scroll() {
 int extent = readByte_t(); if(extent == -1) return;
 int direction = readByte_t(); if(direction == -1) return;
 int movement = readByte_t(); if(movement == -1) return;

 Rect * region;

 switch(extent) {
  case 0:
   region = &textViewport;
   break;
  case 2:
   region = &graphicsViewport;
   break;
  default:
   region = &defaultViewport;
   break;
 }
 Canvas->setScrollingRegion(region->X1, region->Y1, region->X2, region->Y2);

 switch(direction) {
  case 0:
   Canvas->scroll(movement, 0);
   break;
  case 1:
   Canvas->scroll(-movement, 0);
   break;
  case 2:
   Canvas->scroll(0, movement);
   break;
  case 3:
   Canvas->scroll(0, -movement);
   break;
 }
 doWaitCompletion = true;
}



void vdu_sys_cursorBehaviour() {
 int setting = readByte_t(); if(setting == -1) return;
 int mask = readByte_t(); if(mask == -1) return;

 cursorBehaviour = (cursorBehaviour & mask) ^ setting;
}



word play_note(byte channel, byte volume, word frequency, word duration) {
 if(channel >=0 && channel < AUDIO_CHANNELS) {
  return audio_channels[channel]->play_note(volume, frequency, duration);
 }
 return 1;
}



void vdu_sys_sprites(void) {
    uint32_t color;
    void * dataptr;
    int16_t x, y;
    int16_t width, height;
    uint16_t n, temp;

    int cmd = readByte_t();

    switch(cmd) {
     case 0: {
   int rb = readByte_t();
   if(rb >= 0) {
          current_bitmap = rb;
          debug_log("vdu_sys_sprites: bitmap %d selected\n\r", current_bitmap);
   }
  } break;

  case 1:
       case 2: {
   int rw = readWord_t(); if(rw == -1) return;
   int rh = readWord_t(); if(rh == -1) return;

   width = rw;
   height = rh;



         free(bitmaps[current_bitmap].data);



         dataptr = (void *)heap_caps_malloc(sizeof(uint32_t)*width*height, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
         bitmaps[current_bitmap].data = (uint8_t *)dataptr;

         if(dataptr != NULL) {
    if(cmd == 1) {



     for(n = 0; n < width*height; n++) ((uint32_t *)bitmaps[current_bitmap].data)[n] = readLong_b();
     debug_log("vdu_sys_sprites: bitmap %d - data received - width %d, height %d\n\r", current_bitmap, width, height);
    }
    if(cmd == 2) {
     color = readLong_b();



     for(n = 0; n < width*height; n++) ((uint32_t *)dataptr)[n] = color;
     debug_log("vdu_sys_sprites: bitmap %d - set to solid color - width %d, height %d\n\r", current_bitmap, width, height);
    }


    bitmaps[current_bitmap] = Bitmap(width,height,dataptr,PixelFormat::RGBA8888);
    bitmaps[current_bitmap].dataAllocated = false;
   }
         else {



    if (cmd == 1) {
     for(n = 0; n < width*height; n++) readLong_b();
    }
    if (cmd == 2) {
     readLong_b();
    }
    debug_log("vdu_sys_sprites: bitmap %d - data discarded, no memory available - width %d, height %d\n\r", current_bitmap, width, height);
   }
  } break;

  case 3: {
   int rx = readWord_t(); if(rx == -1) return; x = rx;
   int ry = readWord_t(); if(ry == -1) return; y = ry;

   if(bitmaps[current_bitmap].data) {
    Canvas->drawBitmap(x,y,&bitmaps[current_bitmap]);
    doWaitCompletion = true;
   }
   debug_log("vdu_sys_sprites: bitmap %d draw command\n\r", current_bitmap);
  } break;
# 1997 "D:/AgonLight2/agon-vdp/src/video.ino"
     case 4: {
   int b = readByte_t(); if(b == -1) return;
   current_sprite = b;
   debug_log("vdu_sys_sprites: sprite %d selected\n\r", current_sprite);
  } break;

       case 5: {
   sprites[current_sprite].visible = false;
   sprites[current_sprite].setFrame(0);
   sprites[current_sprite].clearBitmaps();
   debug_log("vdu_sys_sprites: sprite %d - all frames cleared\n\r", current_sprite);
  } break;

       case 6: {
   int b = readByte_t(); if(b == -1) return; n = b;
   sprites[current_sprite].addBitmap(&bitmaps[n]);
   debug_log("vdu_sys_sprites: sprite %d - bitmap %d added as frame %d\n\r", current_sprite, n, sprites[current_sprite].framesCount-1);
  } break;

       case 7: {
   int b = readByte_t(); if(b == -1) return;




   numsprites = b;
   if(numsprites) {
    VGAController->setSprites(sprites, numsprites);
   }
   else {
    VGAController->removeSprites();
   }
   doWaitCompletion = true;
   debug_log("vdu_sys_sprites: %d sprites activated\n\r", numsprites);
  } break;

       case 8: {
   sprites[current_sprite].nextFrame();
   debug_log("vdu_sys_sprites: sprite %d next frame\n\r", current_sprite);
  } break;

       case 9: {
   n = sprites[current_sprite].currentFrame;
   if(n) sprites[current_sprite].currentFrame = n-1;
   else sprites[current_sprite].currentFrame = sprites[current_sprite].framesCount - 1;
   debug_log("vdu_sys_sprites: sprite %d previous frame\n\r", current_sprite);
  } break;

       case 10: {
   int b = readByte_t(); if(b == -1) return;
   n = b;
   if(n < sprites[current_sprite].framesCount) sprites[current_sprite].currentFrame = n;
   debug_log("vdu_sys_sprites: sprite %d set to frame %d\n\r", current_sprite,n);
  } break;

       case 11: {
         sprites[current_sprite].visible = 1;
   debug_log("vdu_sys_sprites: sprite %d show cmd\n\r", current_sprite);
  } break;

       case 12: {
   sprites[current_sprite].visible = 0;
   debug_log("vdu_sys_sprites: sprite %d hide cmd\n\r", current_sprite);
  } break;

       case 13: {
   int rx = readWord_t(); if(rx == -1) return; x = rx;
   int ry = readWord_t(); if(ry == -1) return; y = ry;

   sprites[current_sprite].moveTo(x, y);
   debug_log("vdu_sys_sprites: sprite %d - move to (%d,%d)\n\r", current_sprite, x, y);
  } break;

       case 14: {
   int rx = readWord_t(); if(rx == -1) return; x = rx;
   int ry = readWord_t(); if(ry == -1) return; y = ry;
   sprites[current_sprite].x += x;
   sprites[current_sprite].y += y;
   debug_log("vdu_sys_sprites: sprite %d - move by offset (%d,%d)\n\r", current_sprite, x, y);
  } break;

    case 15: {
   if(numsprites) {
    VGAController->refreshSprites();
   }
   debug_log("vdu_sys_sprites: perform sprite refresh\n\r");
  } break;
  case 16: {
   cls(false);
   for(n = 0; n < MAX_SPRITES; n++) {
    sprites[n].visible = false;
    sprites[current_sprite].setFrame(0);
    sprites[n].clearBitmaps();
   }
   for(n = 0; n < MAX_BITMAPS; n++) {
          free(bitmaps[n].data);
    bitmaps[n].dataAllocated = false;
   }
   doWaitCompletion = true;
   debug_log("vdu_sys_sprites: reset\n\r");
  } break;
    }
}