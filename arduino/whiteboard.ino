#include <Adafruit_GFX.h> // Hardware-specific library
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define WHITE    0xFFFF
#define YELLOW   0xFFE0 
#define PINK     0xF81F

#define LANDSCAPE 1

#define DISPLAY_WIDTH   480
#define DISPLAY_HEIGHT  320
#define MENU_WIDTH      40

#define MINPRESSURE 200
#define MAXPRESSURE 1000

#define XP    8
#define XM    A2
#define YP    A3
#define YM    9 

#define TS_LEFT   124
#define TS_RT     900
#define TS_TOP    954
#define TS_BOT    104

#define NUM_BUTTONS 5

#define OPEN_BTN_SIZE 10
#define OPEN_BTN_POS  15

#define MENU_BTN_HEIGHT (DISPLAY_HEIGHT-30)/NUM_BUTTONS

#define SIZE_1 3
#define SIZE_2 2
#define SIZE_3 1

MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

struct point {
  int x, y;
};

enum button_type_e {
  BTN_SIZE =    0x00,
  BTN_RED =     0x01,
  BTN_BLUE =    0x02,
  BTN_REMOVE =  0x03,
  BTN_RESET =   0x04,
  BTN_NONE =    -1,
};

const char* BTN_TYPE_STR[] = {"BTN SIZ", "BTN RED", "BTN BLUE", "BTN REM", "BTN RES", "BTN NON"};

void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, int size);

bool get_touch(struct point* p)
{
  TSPoint touch = ts.getPoint();

  pinMode(YP, OUTPUT);      //restore shared pins
  pinMode(XM, OUTPUT);

  if (touch.z > MINPRESSURE && touch.z < MAXPRESSURE) {

    touch.y = TS_TOP-touch.y;

    p->x = map(touch.y, 0, TS_TOP-TS_BOT, 0, DISPLAY_WIDTH); 
    p->y = map(touch.x, TS_RT, TS_LEFT, 0, DISPLAY_HEIGHT);
    return true;
  }
  return false;
}

void draw_buttons(int color, int pen_size) {

  tft.fillCircle(MENU_WIDTH/2, DISPLAY_HEIGHT/(NUM_BUTTONS+1), MENU_WIDTH/4, WHITE);
  tft.fillCircle(MENU_WIDTH/2, DISPLAY_HEIGHT/(NUM_BUTTONS+1), pen_size*2, BLACK);

  if(color==RED) {
    tft.fillCircle(MENU_WIDTH/2, DISPLAY_HEIGHT/(NUM_BUTTONS+1)*2, MENU_WIDTH/4, WHITE);
    tft.fillCircle(MENU_WIDTH/2, DISPLAY_HEIGHT/(NUM_BUTTONS+1)*2, MENU_WIDTH/5, RED);
  } else {
    tft.fillCircle(MENU_WIDTH/2, DISPLAY_HEIGHT/(NUM_BUTTONS+1)*2, MENU_WIDTH/4, RED);    
  }

  if(color==BLUE) {
    tft.fillCircle(MENU_WIDTH/2, DISPLAY_HEIGHT/(NUM_BUTTONS+1)*3, MENU_WIDTH/4, WHITE);
    tft.fillCircle(MENU_WIDTH/2, DISPLAY_HEIGHT/(NUM_BUTTONS+1)*3, MENU_WIDTH/5, BLUE);
  } else {
    tft.fillCircle(MENU_WIDTH/2, DISPLAY_HEIGHT/(NUM_BUTTONS+1)*3, MENU_WIDTH/4, BLUE);    
  }

  if(color==WHITE) {
    tft.fillCircle(MENU_WIDTH/2, DISPLAY_HEIGHT/(NUM_BUTTONS+1)*4, MENU_WIDTH/4, WHITE);
    tft.fillCircle(MENU_WIDTH/2, DISPLAY_HEIGHT/(NUM_BUTTONS+1)*4, MENU_WIDTH/5, PINK);
  } else {
    tft.fillCircle(MENU_WIDTH/2, DISPLAY_HEIGHT/(NUM_BUTTONS+1)*4, MENU_WIDTH/4, PINK);   
  }

  writeLine(MENU_WIDTH/2-8, DISPLAY_HEIGHT/(NUM_BUTTONS+1)*5 + 8, MENU_WIDTH/2+8, DISPLAY_HEIGHT/(NUM_BUTTONS+1)*5 - 8, WHITE, 2);
  writeLine(MENU_WIDTH/2-8, DISPLAY_HEIGHT/(NUM_BUTTONS+1)*5 - 8, MENU_WIDTH/2+8, DISPLAY_HEIGHT/(NUM_BUTTONS+1)*5 + 8, WHITE, 2);
  
}

void draw_menu(int color, int pen_size) {
  tft.fillRect(0, 0, MENU_WIDTH, DISPLAY_HEIGHT, BLACK);
  draw_buttons(color, pen_size);  
}

enum button_type_e get_pressed_btn(struct point* p) {
  if(p->x<MENU_WIDTH) {
    return (enum button_type_e)map(p->y, 0, DISPLAY_HEIGHT, 0, NUM_BUTTONS);
  } 
  return BTN_NONE;             
} 

void swap(int* a, int* b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}

void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, int size) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(&x0, &y0);
    swap(&x1, &y1);
  }

  if (x0 > x1) {
    swap(&x0, &x1);
    swap(&y0, &y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      tft.fillCircle(y0, x0, size, color);
    } else {
      tft.fillCircle(x0, y0, size, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void setup() {

  Serial.begin(9600);
  
  tft.reset();
  uint16_t identifier = tft.readID();
  
  if (identifier == 0xEFEF) 
    identifier = 0x9486;
  
  tft.begin(identifier);  

}

void loop() {

  tft.setRotation(LANDSCAPE);
  tft.fillScreen(WHITE);

  bool pressed = false;
  bool prec_exists = false;

  int color = RED;
  int pen_size = SIZE_1;

  struct point prec_point;

  draw_menu(color, pen_size);

  for(;;) {

    struct point p;
    if(get_touch(&p)) {
      
      if(p.x<=MENU_WIDTH) {
        if(!pressed) {

          if(prec_exists) {
            tft.fillCircle(prec_point.x, prec_point.y, pen_size, color);
            prec_exists=false;
          }
          
          enum button_type_e btn = get_pressed_btn(&p);
          
          switch(btn) {
            case BTN_RED:
              Serial.print("color F800\n");
              color = RED;
              draw_buttons(color, pen_size);         
              break;        
            case BTN_BLUE:
              Serial.print("color 001F\n");
              color = BLUE; 
              draw_buttons(color, pen_size);        
              break;                   
            case BTN_REMOVE:
              Serial.print("color FFFF\n");
              color = WHITE;     
              draw_buttons(color, pen_size);    
              break;     
            case BTN_RESET:
              Serial.print("clear\n");
              tft.fillRect(MENU_WIDTH, 0, DISPLAY_WIDTH-MENU_WIDTH, DISPLAY_HEIGHT, WHITE);                       
              break;            
            case BTN_SIZE:
              if(pen_size==SIZE_1) {
                pen_size=SIZE_2;  
              } else if(pen_size==SIZE_2) {
                pen_size=SIZE_3;  
              } else {
                pen_size=SIZE_1;  
              }

              draw_buttons(color, pen_size);

              Serial.print("size " + String(pen_size) + "\n");
              break;
          }
          

        }
      } else {
        if(p.x>MENU_WIDTH+5) {
          if(prec_exists && pressed){
            Serial.print("line " + String(p.x-MENU_WIDTH) + " " + String(p.y) + "\n");
            writeLine(prec_point.x,prec_point.y,p.x,p.y,color, pen_size); 
          } else {
            Serial.print("point " + String(p.x-MENU_WIDTH) + " " + String(p.y) + "\n");
            tft.fillCircle(p.x, p.y, pen_size, color);
          }
          if(prec_exists) {
            tft.fillCircle(prec_point.x, prec_point.y, pen_size, color);
            tft.fillCircle(p.x, p.y, pen_size/2, BLACK);
          }
          
          prec_point = p;          
          prec_exists = true;
        }
      }
        
        pressed = true;
      } else {
        pressed = false;
      }
   } 
}
