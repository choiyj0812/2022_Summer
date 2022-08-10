#include "gcode_sample_200_speed.h"

#define beeper  27 //PA4
//----------------------------------
#define X_DIR   21 //PC5
#define X_STEP  15 //PD7
#define XY_EN   14 //PD6
//----------------------------------
#define Y_DIR   23 //PC7
#define Y_STEP  22 //PC6
//----------------------------------
#define X_STOP  18 //PC2
#define Y_STOP  19 //PC3
//----------------------------------
#define ENC_BTN 16 //PC0
#define ENC_A   10 //PD2
#define ENC_B   11 //PD3
//----------------------------------
#define CS      28 //CS/Chip Select - PA3
#define SCK     30 //SCK/Clock - PA1
#define MOSI    17 //MOSI/Master out Slave in - PC1
//==================================
void Setup_X(){
  pinMode(X_DIR, OUTPUT);
  pinMode(X_STEP, OUTPUT);
  pinMode(XY_EN, OUTPUT);
  digitalWrite(X_DIR, LOW);
  digitalWrite(X_STEP, LOW);
  digitalWrite(XY_EN, HIGH);
}

void Setup_Y(){
  pinMode(Y_DIR, OUTPUT);
  pinMode(Y_STEP, OUTPUT);
  pinMode(XY_EN, OUTPUT);
  digitalWrite(Y_DIR, LOW);
  digitalWrite(Y_STEP, LOW);
  digitalWrite(XY_EN, HIGH);
}

#define PI 3.14
unsigned long curr_millis = 0;
unsigned long prev_millis = 0;
unsigned long prev_millis_1 = 0;
unsigned long prev_millis_enc = 0;

unsigned long curr_micros = 0;
unsigned long prev_micros = 0;
unsigned long prev_micros_y = 0;
//----------------------------------
int count_buz = 0;
int duty = 1;
//----------------------------------
uint8_t toggle_motor_x = 0;
volatile int motor_count_x = 0;
volatile uint8_t motor_x_stop = 1;
uint8_t toggle_dir_X = 0;

uint8_t toggle_motor_y = 0;
volatile int motor_count_y = 0;
volatile uint8_t motor_y_stop = 1;
uint8_t toggle_dir_Y = 0;

uint8_t states = 0;
//----------------------------------
uint8_t flag_btn = 0;
enum{
  STATE_0 = 0,
  STATE_1,
  STATE_2,
  STATE_3
};
enum{
  MOTOR_X = 0,
  MOTOR_Y
};
enum{
  MOTOR_MOVE = 0,
  MOTOR_SPEED_SET
};
uint8_t state_enc = 0;
uint8_t select_motor = MOTOR_X;
uint8_t select_mode = MOTOR_MOVE;
unsigned int step_count_x = 400;
unsigned int step_count_y = 400;

int ocr1a_value = 400;  //200 ~ 1000 : 100us ~ 500us
int ocr3a_value = 400;

char *menu[6] = {
  " 1. motor move  ",
  " 2. step count  ",
  " 3. motor speed ",
  " 4. home move   ",
  " 5. auto start  ",
  " 6. haha        "
};

enum{
  MENU = 0,
  SUB_MENU
};
enum{
  SUB_MOTOR_MOVE = 0,
  SUB_MOTOR_STEP_COUNT,
  SUB_MOTOR_SPEED,
  SUB_HOME,
  SUB_AUTO_START,
  SUB_HOHO
};

//------------------------------------

float prev_point_x = 0, prev_point_y = 0;
int pos_xy_index = 0;

int lcd_window_index = 0;
int lcd_cursor_index = 0;
int prev_lcd_cursor_index = 0;
int selected_menu_index = 0;

void setup() {
  Serial.begin(9600);
  //----------------------------------
  pinMode(beeper, OUTPUT);
  digitalWrite(beeper, LOW);
  //----------------------------------
  pinMode(X_STOP, INPUT);
  pinMode(Y_STOP, INPUT);
  //----------------------------------
  pinMode(ENC_BTN, INPUT);
  pinMode(ENC_A, INPUT);
  pinMode(ENC_B, INPUT);
  digitalWrite(ENC_BTN, HIGH);
  digitalWrite(ENC_A, HIGH);
  digitalWrite(ENC_B, HIGH);
  //----------------------------------
  pinMode(CS, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(MOSI, OUTPUT);
  //=======================================================
  //ENCODER BUTTON
  /*while(1){
    //----------------------------------
    func_btn_enc();
    //----------------------------------
    delay(10);
  }*/
  
  //=======================================================
  //Timer2
  /*TCCR2A = 0x00;  // Normal Mode
  TCCR2B = 0x03;  // 0x02 : 8분주, 0x03 : 32분주
  TCNT2 = 156;
  TIMSK2 = 0x01;  // 0x01 : Overflow Interrupt*/

  //=======================================================
  //Timer1
  /*TCCR1A = 0x00;
  TCCR1B = 0x02;  //0x02 : 8분주 = 0.5us
  TCCR1C = 0x00;
  TCNT1 = 65536 - 400;
  //400 : 400 * 0.5us => 200us마다 인터럽트 발생
  //65536 = 2^16
  TIMSK1 = 0x01;*/

  TCCR1A = 0x00;
  TCCR1B = 0x0A;  //0x02 : 8분주 = 0.5us
  TCCR1C = 0x00;
  TCNT1 = 0;
  OCR1A = ocr1a_value;
  TIMSK1 = 0x02;
  //0x01 : overflow interrupt, 0x02 : compare match interrupt A

  //Timer3
  TCCR3A = 0x00;
  TCCR3B = 0x0A;  //0x02 : 8분주 = 0.5us
  TCCR3C = 0x00;
  TCNT3 = 0;
  OCR3A = ocr3a_value;
  TIMSK3 = 0x02;
  //0x01 : overflow interrupt, 0x02 : compare match interrupt A
  
  //=======================================================
  Setup_X();
  Setup_Y();
  digitalWrite(X_DIR, LOW);
  digitalWrite(Y_DIR, LOW);
  digitalWrite(XY_EN, LOW);

  //digitalWrite(X_DIR, HIGH);
  //digitalWrite(Y_DIR, HIGH);

  //=======================================================
  digitalWrite(XY_EN, LOW);

  /*TCNT1 = 0;
  OCR1A = ocr1a_value;
  step_count_x = 4000;
  motor_x_stop = 0;

  TCNT3 = 0;
  OCR3A = ocr3a_value;
  step_count_y = 4000;
  motor_y_stop = 0;*/
  
  /*p2p_move(50,0);
  delay(3000);
  p2p_move(50,50);
  delay(3000);
  p2p_move(0,50);
  delay(3000);
  p2p_move(0,0);
  delay(3000);*/
  
  /*double pos_xy[4][2] = {{50, 0}, {50, 50}, {0, 50}, {0, 0}};
  for(int i=0;i<4;i++){
    p2p_move(pos_xy[i][0], pos_xy[i][1]);
    delay(3000);
  }*/

  /*double pos_xy[4][2] = {{50, 0}, {50, 50}, {0, 50}, {0, 0}};
  while(1){
    if(motor_x_stop == 1 && motor_y_stop == 1){
      p2p_move(pos_xy[pos_xy_index][0], pos_xy[pos_xy_index][1]);
      pos_xy_index++;
      if(pos_xy_index == 4) break;
    }
    delay(1);
  }*/
  /*while(1){
    if(motor_x_stop == 1 && motor_y_stop == 1){
      p2p_move(xy_pos[pos_xy_index][0], xy_pos[pos_xy_index][1]);
      pos_xy_index++;
      if(pos_xy_index == 480) break;
      //Serial.println(pos_xy_index);
    }
    //delay(1);
  }*/
  //=======================================================
  //LCD
  //명령어 배열
  uint8_t command_array[8] = {0x30, 0x30, 0x30, 0x38, 0x06, 0x0C, 0x80, 0x01};
  for(int i=0;i<8;i++){
    lcd_command_set(command_array[i]);
  }
  delay(2);

  lcd_window_index = 0;

  lcd_print_all_menu(lcd_window_index);
  lcd_set_print_cursor(0);
  /*lcd_set_cursor(0,0);
  lcd_set_string("haha");
  lcd_set_cursor(1,0);
  lcd_set_string("Hello World!");

  int count_lcd = 0;
  char buf[20];
  while(1){
    sprintf(buf, "%d", count_lcd++);
    lcd_set_cursor(2,0);
    lcd_set_string(buf);
    delay(1000);
  }*/
  //=======================================================
  //SWITCH
  
  while(1){
    /*if(digitalRead(X_STOP) == 1){
      motor_x_stop = 1;
    }
    if(digitalRead(Y_STOP) == 1){
      motor_y_stop = 1;
    }*/
    curr_millis = millis();
    //curr_micros = micros();

    if(curr_millis - prev_millis_enc > 1){
      prev_millis_enc = curr_millis;

      func_btn_enc();
    }
    
    /*if(curr_millis - prev_millis > 1000){
      prev_millis = curr_millis;
      Serial.println("1sec");
    }*/

    /*if(curr_millis - prev_millis_1 > 10){
      prev_millis_1 = curr_millis;
      duty++;
      if(duty == 99) duty = 1;
    }
    if(curr_micros - prev_micros > 100){
      count_buz++;
      if(count_buz == 100){
        count_buz = 0;
        digitalWrite(beeper, HIGH);
      }
      else if(count_buz == duty) digitalWrite(beeper, LOW);
    }*/

    //-------------------------------------

    /*if(curr_millis - prev_millis_1 > 3000){
      prev_millis_1 = curr_millis;

//      if(states == 0){
//        digitalWrite(X_DIR, LOW);
//        motor_x_stop = 0;
//        motor_y_stop = 1;
//      }
//      else if(states == 1){
//        digitalWrite(Y_DIR, LOW);
//        motor_x_stop = 1;
//        motor_y_stop = 0;
//      }
//      else if(states == 2){
//        digitalWrite(X_DIR, HIGH);
//        motor_x_stop = 0;
//        motor_y_stop = 1;
//      }
//      else{
//        digitalWrite(Y_DIR, HIGH);
//        motor_x_stop = 1;
//        motor_y_stop = 0;
//      }
//
//      states++;
//      if(states == 4) states = 0;

      //-------------------------------------
      
      if(toggle_dir_X == 0){
        toggle_dir_X = 1;
        digitalWrite(X_DIR, LOW);
        motor_x_stop = 0;
      }
      else{
        toggle_dir_X = 0;
        digitalWrite(X_DIR, HIGH);
        motor_x_stop = 0;
      }

      if(toggle_dir_Y == 0){
        toggle_dir_Y = 1;
        digitalWrite(Y_DIR, LOW);
        motor_y_stop = 0;
      }
      else{
        toggle_dir_Y = 0;
        digitalWrite(Y_DIR, HIGH);
        motor_y_stop = 0;
      }
    }*/

    /*if(motor_x_stop == 0){
      if(curr_micros - prev_micros > 200){
        prev_micros = curr_micros;
        if(toggle_motor_x == 0){
          toggle_motor_x = 1;
          digitalWrite(X_STEP, HIGH);
  
          motor_count_x++;
          if(motor_count_x == 400){
            motor_count_x = 0;
            motor_x_stop = 1;
          }
        }
        else{
          toggle_motor_x = 0;
          digitalWrite(X_STEP, LOW);
        }
      }
    }

    if(motor_y_stop == 0){
      if(curr_micros - prev_micros_y > 200){
        prev_micros_y = curr_micros;
        if(toggle_motor_y == 0){
          toggle_motor_y = 1;
          digitalWrite(Y_STEP, HIGH);
  
          motor_count_y++;
          if(motor_count_y == 400){
            motor_count_y = 0;
            motor_y_stop = 1;
          }
        }
        else{
          toggle_motor_y = 0;
          digitalWrite(Y_STEP, LOW);
        }
      }
    }*/
  }


  
  //=======================================================
  /*double radian = 0;
  double degree_1 = 60.0;
  double degree_to_radian = ((degree_1 / 360.0) * (2 * PI));
  double distance_b = 5 * cos(degree_to_radian);
  double distance_c = 5 * sin(degree_to_radian);
  Serial.println(distance_b);
  Serial.println(distance_c);
  
  while(1);*/
  //=======================================================
  /*Setup_X();
  Setup_Y();
  Active_X(true);
  Active_XY(false, true, 1, 2);
  Active_XY(false, false, 1, 2);*/
}

void loop() {
  
}

void lcd_set_print_cursor(int cursor_line_num){
  lcd_set_cursor(prev_lcd_cursor_index,0);
  lcd_data_set(' ');
  
  lcd_set_cursor(cursor_line_num,0);
  lcd_data_set('>');
  prev_lcd_cursor_index = cursor_line_num;
}

void lcd_print_all_menu(int index){
  if(index <= 2 && index >= 0){
    for(int i=0;i<4;i++){
      lcd_set_cursor(i,0);
      lcd_set_string(menu[i + index]);
    }
  }
}

void lcd_set_cursor(unsigned int line_num, int col){
  if(line_num == 0){
    if(col < 8){
      lcd_command_set(0x80 + col);
    }
  }
  else if(line_num == 1){
    if(col < 8){
      lcd_command_set(0x90 + col);
    }
  }
  else if(line_num == 2){
    if(col < 8){
      lcd_command_set(0x88 + col);
    }
  }
  else if(line_num == 3){
    if(col < 8){
      lcd_command_set(0x98 + col);
    }
  }
}

void lcd_set_string(char *str){
  int str_len = strlen(str);

  for(int i=0;i<str_len;i++){
    lcd_data_set(str[i]); // = lcd_data_set(*(str + i));
  }
}

void lcd_command_set(uint8_t command){
  digitalWrite(CS, HIGH);
  uint8_t upper_byte = command & 0xF0;
  uint8_t lower_byte = (command & 0x0F) << 4;
  shift_8bit(0xF8); //0xF8 : command
  shift_8bit(upper_byte);
  shift_8bit(lower_byte);
  digitalWrite(CS, LOW);
  delayMicroseconds(50);
}

void lcd_data_set(uint8_t lcd_data){
  digitalWrite(CS, HIGH);
  uint8_t upper_byte = lcd_data & 0xF0;
  uint8_t lower_byte = (lcd_data & 0x0F) << 4;
  shift_8bit(0xFA); //0xFA : data
  shift_8bit(upper_byte);
  shift_8bit(lower_byte);
  digitalWrite(CS, LOW);
  delayMicroseconds(50);
}

void shift_8bit(uint8_t shift_data){
  for(int i=0;i<8;i++){
    if(shift_data & (0x80 >> i)){
      digitalWrite(MOSI, HIGH);
    }
    else{
      digitalWrite(MOSI, LOW);
    }
    digitalWrite(SCK, HIGH);
    digitalWrite(SCK, LOW);
  }
}

/*void p2p_move(float dest_x, float dest_y){  //dest_x(mm), dest_y(mm)
  float diff_x = dest_x - prev_point_x;
  float diff_y = dest_y - prev_point_y;
//  Serial.print("diff_x : ");
//  Serial.println(diff_x);
//  Serial.print("diff_y : ");
//  Serial.println(diff_y);
  
  step_count_x = 0;
  step_count_y = 0;
  OCR1A = 400;
  OCR3A = 400;

  //Serial.print("X_DIR : ");
  if(diff_x > 0){
    digitalWrite(X_DIR, LOW);
    //Serial.print("+, ");
  }
  //else if(diff_x == 0) Serial.print("0, ");
  else{
    digitalWrite(X_DIR, HIGH);
    //Serial.print("-, ");
  }
  
  //Serial.print("Y_DIR : ");
  if(diff_y > 0){
    digitalWrite(Y_DIR, LOW);
    //Serial.println("+");
  }
  //else if(diff_y == 0) Serial.println("0, ");
  else{
    digitalWrite(Y_DIR, HIGH);
    //Serial.println("-");
  }
  //------------------------------------------
  if(diff_x == 0 && diff_y == 0) return;
  else if(diff_x != 0 && diff_y != 0){
    ocr1a_value = 400;                              // x
    ocr3a_value = (int)round(abs((400 * (diff_x / diff_y)))); // y

    step_count_x = (int)round(abs((diff_x * 80))); //80 = 1 / 0.0125(mm)
    step_count_y = (int)round(abs((diff_y * 80)));

    TCNT1 = 0;
    TCNT3 = 0;

    OCR1A = ocr1a_value;
    OCR3A = ocr3a_value;

    motor_x_stop = 0;
    motor_y_stop = 0;
  }
  else if(diff_y != 0){
    ocr3a_value = 400;
    step_count_y = (int)round(abs((diff_y * 80)));
    TCNT3 = 0;
    OCR3A = ocr3a_value;
    motor_x_stop = 1;
    motor_y_stop = 0;
  }
  else if(diff_x != 0){
    ocr1a_value = 400;
    step_count_x = (int)round(abs((diff_x * 80)));
    TCNT1 = 0;
    OCR1A = ocr1a_value;
    motor_x_stop = 0;
    motor_y_stop = 1;
  }
//  Serial.print("step_count_x : ");
//  Serial.println(step_count_x);
//  Serial.print("step_count_y : ");
//  Serial.println(step_count_y);
  
  //------------------------------------------
  prev_point_x = dest_x;
  prev_point_y = dest_y;
}*/

void p2p_move_speed(float dest_x, float dest_y, int speed_half_period){  //dest_x(mm), dest_y(mm)
  float diff_x = dest_x - prev_point_x;
  float diff_y = dest_y - prev_point_y;
//  Serial.print("diff_x : ");
//  Serial.println(diff_x);
//  Serial.print("diff_y : ");
//  Serial.println(diff_y);
  
  step_count_x = 0;
  step_count_y = 0;
  OCR1A = 400;
  OCR3A = 400;

  //Serial.print("X_DIR : ");
  if(diff_x > 0){
    digitalWrite(X_DIR, LOW);
    //Serial.print("+, ");
  }
  //else if(diff_x == 0) Serial.print("0, ");
  else{
    digitalWrite(X_DIR, HIGH);
    //Serial.print("-, ");
  }
  
  //Serial.print("Y_DIR : ");
  if(diff_y > 0){
    digitalWrite(Y_DIR, LOW);
    //Serial.println("+");
  }
  //else if(diff_y == 0) Serial.println("0, ");
  else{
    digitalWrite(Y_DIR, HIGH);
    //Serial.println("-");
  }
  //------------------------------------------
  if(diff_x == 0 && diff_y == 0) return;
  else if(diff_x != 0 && diff_y != 0){
    ocr1a_value = 400;                              // x
    ocr3a_value = (int)round(abs((400 * (diff_x / diff_y)))); // y

    step_count_x = (int)round(abs((diff_x * 80))); //80 = 1 / 0.0125(mm)
    step_count_y = (int)round(abs((diff_y * 80)));

    //------------------------------------------
    //speed 계산
    //Theta 계산
    double theta = atan2(diff_y, diff_x);
    //x speed 계산
    int half_period_x = (int)round(abs((speed_half_period * (1 / cos(theta)))));
    //y speed 계산
    int half_period_y = (int)round(abs((speed_half_period * (1 / sin(theta)))));
    //------------------------------------------

    TCNT1 = 0;
    TCNT3 = 0;

    OCR1A = half_period_x;
    OCR3A = half_period_y;

    motor_x_stop = 0;
    motor_y_stop = 0;
  }
  else if(diff_y != 0){
    ocr3a_value = 400;
    step_count_y = (int)round(abs((diff_y * 80)));
    //------------------------------------------
    //speed 계산
    //Theta 계산
    double theta = atan2(diff_y, diff_x);
    //y speed 계산
    int half_period_y = (int)round(abs((speed_half_period * (1 / sin(theta)))));
    //------------------------------------------
    TCNT3 = 0;
    OCR3A = half_period_y;
    motor_x_stop = 1;
    motor_y_stop = 0;
  }
  else if(diff_x != 0){
    ocr1a_value = 400;
    step_count_x = (int)round(abs((diff_x * 80)));
    //------------------------------------------
    //speed 계산
    //Theta 계산
    double theta = atan2(diff_y, diff_x);
    //x speed 계산
    int half_period_x = (int)round(abs((speed_half_period * (1 / cos(theta)))));
    //------------------------------------------
    TCNT1 = 0;
    OCR1A = half_period_x;
    motor_x_stop = 0;
    motor_y_stop = 1;
  }
  /*Serial.print("step_count_x : ");
  Serial.println(step_count_x);
  Serial.print("step_count_y : ");
  Serial.println(step_count_y);*/
  
  //------------------------------------------
  prev_point_x = dest_x;
  prev_point_y = dest_y;
}

uint8_t toggle_btn = 0;
int menu_index = 0;
int btn_select = 0;
int which_menu = 0;

void func_btn_enc(){
  //int enc_btn = digitalRead(ENC_BTN);
  uint8_t pin_c = PINC;
  int enc_btn = (pin_c & 0x01) ? 1 : 0;
  // pullup -> 0:push
  if(enc_btn == 0 && flag_btn == 0){
    flag_btn = 1;
    //Serial.println("push");

    /*if(toggle_btn == 0){
      toggle_btn = 1;
      select_motor = MOTOR_X;
      Serial.println("MOTOR_X");
    }
    else if(toggle_btn == 1){
      toggle_btn = 0;
      select_motor = MOTOR_Y;
      Serial.println("MOTOR_Y");
    }*/
    //-----------------------------
    /*if(toggle_btn == 0){
      toggle_btn = 1;
      select_mode = MOTOR_MOVE;
      Serial.println("now mode : MOTOR_MOVE");
    }
    else if(toggle_btn == 1){
      toggle_btn = 0;
      select_mode = MOTOR_SPEED_SET;
      Serial.println("MOTOR_SPEED_SET");
    }*/
    //-------------------------------------------
    /*if(which_menu == MENU){
      btn_select = menu_index;
      which_menu = SUB_MENU;
      Serial.println("Select this SUB_MENU!");

      if(btn_select == SUB_HOME){
        step_count_x = 30000;
        digitalWrite(X_DIR, HIGH);
        motor_x_stop = 0;

        step_count_y = 30000;
        digitalWrite(Y_DIR, HIGH);
        motor_y_stop = 0;

        while(1){
          int x_stop = digitalRead(X_STOP);
          int y_stop = digitalRead(Y_STOP);
          Serial.println(x_stop);
          Serial.println(y_stop);
          if(x_stop == 1){
            motor_x_stop = 1;
          }
          if(y_stop == 1){
            motor_y_stop = 1;
          }
          if(motor_x_stop == 1 && motor_y_stop == 1){
            step_count_x = 400;
            step_count_y = 400;
            break;
          }
          delay(1);
        }
      }
    }
    else if(which_menu == SUB_MENU){
      which_menu = MENU;
      Serial.println("Return to MAIN_MENU!");
    }*/
    //----------------------------------
    selected_menu_index = lcd_window_index + lcd_cursor_index;
    //Serial.println(menu[selected_menu_index]);
    
    if(selected_menu_index == SUB_HOME){
      step_count_x = 30000;
      digitalWrite(X_DIR, HIGH);
      motor_x_stop = 0;

      step_count_y = 30000;
      digitalWrite(Y_DIR, HIGH);
      motor_y_stop = 0;

      while(1){
        int x_stop = digitalRead(X_STOP);
        int y_stop = digitalRead(Y_STOP);
        //Serial.println(x_stop);
        //Serial.println(y_stop);
        if(x_stop == 1){
          motor_x_stop = 1;
        }
        if(y_stop == 1){
          motor_y_stop = 1;
        }
        if(motor_x_stop == 1 && motor_y_stop == 1){
          step_count_x = 400;
          step_count_y = 400;
          break;
        }
        //delay(1);
      }
    }
    else if(selected_menu_index == SUB_AUTO_START){
      digitalWrite(X_STEP, LOW);
      digitalWrite(Y_STEP, LOW);
      digitalWrite(XY_EN, LOW);
      motor_x_stop = 1;
      motor_y_stop = 1;
      
      /*while(1){
        if(motor_x_stop == 1 && motor_y_stop == 1){
          p2p_move(xy_pos[pos_xy_index][0], xy_pos[pos_xy_index][1]);
          pos_xy_index++;
          if(pos_xy_index == 480) break;
          //Serial.println(pos_xy_index);
        }
        //delay(1);
      }*/
      //-------------------------------------------------------------
      while(1){
        if(motor_x_stop == 1 && motor_y_stop == 1){
          p2p_move_speed(xy_pos[pos_xy_index][0], xy_pos[pos_xy_index][1], (int)xy_pos[pos_xy_index][2]);
          pos_xy_index++;
          if(pos_xy_index == 200) break;
          //Serial.println(pos_xy_index);
        }
        //delay(1);
      }
    }
  }
  else if(enc_btn == 1 && flag_btn == 1){
    flag_btn = 0;
    //Serial.println("pull");
  }
  //----------------------------------
  int enc_a = digitalRead(ENC_A);
  int enc_b = digitalRead(ENC_B);

  //Serial.print(enc_a);  Serial.print(' ');
  //Serial.println(enc_b);
  if(enc_a == 1 && enc_b == 1){
    state_enc = STATE_0;
  }
  else if(enc_a == 0 && enc_b == 1){
    if(state_enc == STATE_2){
      //Serial.println("Left");

      /*if(select_motor == MOTOR_X){
        digitalWrite(X_DIR, HIGH);
        motor_x_stop = 0;
      }
      else{
        digitalWrite(Y_DIR, HIGH);
        motor_y_stop = 0;
      }*/
      //------------------------------
      /*if(select_mode == MOTOR_MOVE){
        digitalWrite(X_DIR, HIGH);
        motor_x_stop = 0;
      }
      else if(select_mode == MOTOR_SPEED_SET){
//        step_count -= 100;
//        if(step_count < 200) step_count = 200;
//        Serial.println(step_count);
          //--------------------------------------
          TCNT1 = 0;
          ocr1a_value -= 100;
          if(ocr1a_value < 200) ocr1a_value = 200;
          OCR1A = ocr1a_value;
          Serial.println(ocr1a_value);
      }*/
      //------------------------------
      /*if(which_menu == MENU){
        menu_index--;
        if(menu_index < 0) menu_index = 4;
        Serial.println(menu[menu_index]);
      }
      else if(which_menu == SUB_MENU){
        if(btn_select == SUB_MOTOR_MOVE){
          digitalWrite(X_DIR, HIGH);
          motor_x_stop = 0;
        }
        else if(btn_select == SUB_MOTOR_STEP_COUNT){
          step_count_x -= 100;
          if(step_count_x < 200) step_count_x = 200;
          Serial.println(step_count_x);
        }
        else if(btn_select == SUB_MOTOR_SPEED){
          TCNT1 = 0;
          ocr1a_value -= 100;
          if(ocr1a_value < 200) ocr1a_value = 200;
          OCR1A = ocr1a_value;
          Serial.println(ocr1a_value);
        }
      }*/
      //------------------------------
      /*lcd_window_index--;
      if(lcd_window_index < 0) lcd_window_index = 0;
      lcd_print_all_menu(lcd_window_index);*/
      //------------------------------
      lcd_cursor_index--;
      if(lcd_cursor_index < 0){
        lcd_cursor_index = 0;
        
        lcd_window_index--;
        if(lcd_window_index < 0) lcd_window_index = 0;
        lcd_print_all_menu(lcd_window_index);
      }
      lcd_set_print_cursor(lcd_cursor_index);
    }
    state_enc = STATE_1;
  }
  else if(enc_a == 0 && enc_b == 0){
    if(state_enc == STATE_1){
      //Serial.println("Right");

      /*if(select_motor == MOTOR_X){
        digitalWrite(X_DIR, LOW);
        motor_x_stop = 0;
      }
      else{
        digitalWrite(Y_DIR, LOW);
        motor_y_stop = 0;
      }*/
      //------------------------------
      /*if(select_mode == MOTOR_MOVE){
        digitalWrite(X_DIR, LOW);
        motor_x_stop = 0;
      }
      else if(select_mode == MOTOR_SPEED_SET){
//        step_count += 100;
//        if(step_count > 4000) step_count = 4000;
//        Serial.println(step_count);
          //------------------------------------------
          TCNT1 = 0;
          ocr1a_value += 100;
          if(ocr1a_value > 4000) ocr1a_value = 4000;
          OCR1A = ocr1a_value;
          Serial.println(ocr1a_value);
      }*/
      //------------------------------
      /*if(which_menu == MENU){
        menu_index++;
        if(menu_index > 4) menu_index = 0;
        Serial.println(menu[menu_index]);
      }
      else if(which_menu == SUB_MENU){
        if(btn_select == SUB_MOTOR_MOVE){
          digitalWrite(X_DIR, LOW);
          motor_x_stop = 0;
        }
        else if(btn_select == SUB_MOTOR_STEP_COUNT){
          step_count_x += 100;
          if(step_count_x > 4000) step_count_x = 4000;
          Serial.println(step_count_x);
        }
        else if(btn_select == SUB_MOTOR_SPEED){
          TCNT1 = 0;
          ocr1a_value += 100;
          if(ocr1a_value > 4000) ocr1a_value = 4000;
          OCR1A = ocr1a_value;
          Serial.println(ocr1a_value);
        }
      }*/
      //------------------------------
      /*lcd_window_index++;
      if(lcd_window_index > 2) lcd_window_index = 2;
      lcd_print_all_menu(lcd_window_index);*/
      //------------------------------
      lcd_cursor_index++;
      if(lcd_cursor_index > 3){
        lcd_cursor_index = 3;
        
        lcd_window_index++;
        if(lcd_window_index > 2) lcd_window_index = 2;
        lcd_print_all_menu(lcd_window_index);
      }
      lcd_set_print_cursor(lcd_cursor_index);
    }
    state_enc = STATE_2;
  }
  else if(enc_a == 1 && enc_b == 0){
    state_enc = STATE_3;
  }
  //Serial.println(state_enc);
}

//SIGNAL(TIMER2_OVF_vect){
//SIGNAL(TIMER1_OVF_vect){
SIGNAL(TIMER1_COMPA_vect){
  //TCNT1 = 65536 - 400;
  
  if(motor_x_stop == 0){
    if(toggle_motor_x == 0){
      toggle_motor_x = 1;
      digitalWrite(X_STEP, HIGH);
  
      motor_count_x++;
      if(motor_count_x == step_count_x){
        motor_count_x = 0;
        motor_x_stop = 1;
      }
    }
    else{
      toggle_motor_x = 0;
      digitalWrite(X_STEP, LOW);
    }
  }
  /*else if(motor_x_stop + motor_y_stop == 2){
    digitalWrite(X_DIR, LOW);
    if(toggle_motor_x == 0){
      toggle_motor_x = 1;
      digitalWrite(X_STEP, HIGH);
  
      motor_count_x++;
      if(motor_count_x == 4000){
        motor_count_x = 0;
        motor_x_stop = 2;
      }
    }
    else{
      toggle_motor_x = 0;
      digitalWrite(X_STEP, LOW);
    }
  }*/
}

SIGNAL(TIMER3_COMPA_vect){
  //TCNT1 = 65536 - 400;
  
  if(motor_y_stop == 0){
    if(toggle_motor_y == 0){
      toggle_motor_y = 1;
      digitalWrite(Y_STEP, HIGH);
  
      motor_count_y++;
      if(motor_count_y == step_count_y){
        motor_count_y = 0;
        motor_y_stop = 1;
      }
    }
    else{
      toggle_motor_y = 0;
      digitalWrite(Y_STEP, LOW);
    }
  }
  /*else if(motor_x_stop + motor_y_stop == 2){
    digitalWrite(Y_DIR, LOW);
    if(toggle_motor_y == 0){
      toggle_motor_y = 1;
      digitalWrite(Y_STEP, HIGH);
  
      motor_count_y++;
      if(motor_count_y == 4000){
        motor_count_y = 0;
        motor_y_stop = 2;
      }
    }
    else{
      toggle_motor_y = 0;
      digitalWrite(Y_STEP, LOW);
    }
  }*/
}
