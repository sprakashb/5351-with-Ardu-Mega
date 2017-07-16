/* Arduino Sketch to control Si5351 for generating VFO and BFO signals for Bitx or any multiband transceiver
    Uses MCUFRIEND 2.4 inch display with touch screen for controlling all functions. The display and a extension board sit piggy back on the Arduino Mega board.
    SP Bhatnagar VU2SPF ,  vu2spf@gmail.com
  Released under GNU GPL3.0
  The Author VU2SPF respectfully and humbly acknowledes the fine contributions of authors of various libraries and other similar projects for publishing their
  code for the benefit of HAMS and other electronic enthusiasts. Ideas from many of these may reflect in this work. Heartfelt Thanks to all users and testers for encouragement.
  There are lot of possible additions / upgrades and improvements. Please suggest or correct and publish your changes for the benefit of all HAMS.
*/
#define Ver "V1.06"     // 16/7/17 Corrected band change, Sideband flip and bfo display problems
//#define Ver "V1.05"   // 7/7/17 based on practical experimentations wih Bitx acive areas in buttons shrunk from all sides by few pixels.
//#define Ver "V1.04"   //   15th June Tidied up display and added 4 fn buttons on 5th row To be assigned later
//#define Ver "V1.03"  // 14th June 2017 Optimization and shorter band table
//#define Ver "V1.02"  // 9th June 2017 , Optimized prog and minor changes
//#define Ver "V1.01"  // 2nd June 2017 Tested with 5351 board and Multiband Bitx

#include <EEPROM.h>
#include "EEPROMAnything.h"
#include <Rotary.h>
#include <avr/io.h>
#include <si5351.h>
#include <Wire.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>
#include "TouchScreen.h"

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Assign human-readable names to some common 16-bit color values:
#define BLACK       0x0000      /*   0,   0,   0 */
#define LIGHTGREY   0xC618      /* 192, 192, 192 */
#define GREY        0x7BEF      /* 128, 128, 128 */
#define BLUE        0x001F      /*   0,   0, 255 */
#define RED         0xF800      /* 255,   0,   0 */
#define YELLOW      0xFFE0      /* 255, 255,   0 */
#define WHITE       0xFFFF      /* 255, 255, 255 */
#define ORANGE      0xFD20      /* 255, 165,   0 */
#define GREEN       0x07E0
#define CYAN        0x07FF
#define MAGENTA     0xF81F

// most mcufriend shields use these pins and Portrait mode:
uint8_t YP = A1;  // must be an analog pin, use "An" notation!
uint8_t XM = A2;  // must be an analog pin, use "An" notation!
uint8_t YM = 7;   // can be a digital pin
uint8_t XP = 6;   // can be a digital pin

uint16_t TS_LEFT = 950;
uint16_t TS_RT  = 120;
uint16_t TS_TOP = 920;
uint16_t TS_BOT = 120;

// defines for input buttons (if any)
#define BandSelect      53
#define SideBandSelect  52

#define ENCODER_A    67   // Encoder pin A is A13 on Mega
#define ENCODER_B    68  // Encoder pin B is A14 on Mega
#define ENCODER_BTN  37
#define PTT_Input 16      // first pin on top towards arduino
//#define TEST_OP 32  // for profiling/testing the time spent in diff routines check on CRO
//boolean test_op = HIGH;

// defines for output controls - PTT
#define PTT_Output 17       // second pin on top

Si5351 si5351;
Rotary r = Rotary(ENCODER_A, ENCODER_B);
MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint tp;
#define MINPRESSURE 20
#define MAXPRESSURE 1000

// uint16_t identifier;
// uint8_t Orientation = 1;    //LS

volatile uint32_t bfo = 9996000UL;  // should be selected based on txcvr
volatile uint32_t bfo_USB = 9994000UL , bfo_LSB = 9996000UL;   // all bfos are tunable and saved on EEPROM
volatile uint32_t bfo_A, bfo_B, bfo_M;
//The bfo frequency is added to or subtracted from the vfo frequency in the "Loop()"
volatile uint32_t vfo , vfo_tx, if_offset = 1700;
volatile uint32_t vfo_A = 7050000UL;  // temp values
volatile uint32_t vfo_B = 7130000UL;
volatile uint32_t vfo_M = 14000000UL;
// either vfo A or B or mem channel is selected for output at any time
boolean vfo_A_sel = true; // true when vfo a selected
boolean vfo_B_sel = false; // for selecting vfo b as active
boolean vfo_M_sel = false ;  // true when memCh is selected
boolean changed_f = 0;
boolean xch_M = 0; // flag for xchged mem in V > M
uint16_t sideband, sb_A, sb_B, sb_M;
uint16_t LSB = 1, USB = 2;
// display step size and radix
String step_sz_txt[] = {"   1Hz ", "   10Hz ", "  100Hz ", "   1kHz ", "  10kHz ", "  100kHz", "   1MHz "};
uint32_t step_sz[] = {    1,          10,        100,        1000,     10000,      100000,     1000000};
int step_index = 3;
uint32_t radix = 1000;  //start step size - change to suit

//$$$$$ mem related
uint16_t max_memory_ch = 100;  // each ch takes 10 bytes
struct allinfo {
  uint32_t s_vfo;
  uint32_t s_bfo;
  uint16_t s_sb;
} Allinfo;  // complete description of channel saved in mem

uint16_t eprom_base_addr = 0;
// Eprom content : allinfo for vfoA, vfoB, mem1,2,3...
uint16_t address;  // temp address used y fns
allinfo ch_info;    // local copy
unsigned int memCh = 1;  // memory  channel  number (1,2,3...100) now.  Try names..

const int SENSOR = 66;  // Analog Pin A12 for the S Meter function  $$$
int val = 0;
int val1 = 0;          // s meter variables
int val2 = 0;
int val5 = 0;
int val6 = 0;    ///##############################################

unsigned long debounce, DebounceDelay = 400000;

int bnd_count, old_band;
int MAX_BANDS = 9;
volatile uint32_t F_MIN_T[9] = {100000UL,  3500000UL, 7000000UL, 10100000UL, 14000000UL, 18068000UL, 21000000UL, 24890000UL, 28000000UL};
volatile uint32_t  F_MAX_T[9] = {75000000UL,  3800000UL, 7200000UL, 10150000UL, 14350000UL, 18168000UL, 21450000UL, 24990000UL, 29700000UL};
String  B_NAME_T[] = {"  VFO", "  80m", "  40m", "  30m", "  20m", "  17m", "  15m", "  12m", "  10m" };
volatile uint32_t  VFO_T[9] = {9500000UL, 3670000UL, 7100000UL, 10120000UL, 14200000UL, 18105000UL, 21200000UL, 24925000UL, 28500000UL};
int band_cntrl[] = {23, 24, 25, 26, 16, 28, 29, 30, 31}; // pins for controlling BPF and LPF corresponding to each band (20m 16 was 27)

/* all bands
  volatile uint32_t F_MIN_T[13] = {100000UL, 1810000UL, 3500000UL, 7000000UL, 10100000UL, 14000000UL, 18068000UL, 21000000UL, 24890000UL, 28000000UL, 50000000UL, 70000000UL, 5398500UL};
  volatile uint32_t  F_MAX_T[13] = {75000000UL, 2000000UL, 3800000UL, 7200000UL, 10150000UL, 14350000UL, 18168000UL, 21450000UL, 24990000UL, 29700000UL, 52000000UL, 70500000UL, 5403500UL};
  String  B_NAME_T[] = {"  VFO", " 160m", "  80m", "  40m", "  30m", "  20m", "  17m", "  15m", "  12m", "  10m", "   6m", "   4m", "  60m"};
  volatile uint32_t  VFO_T[13] = {9500000UL, 1850000UL, 3670000UL, 7100000UL, 10120000UL, 14200000UL, 18105000UL, 21200000UL, 24925000UL, 28500000UL, 50150000UL, 70150000UL, 5398500UL};
*/
//#########################################
unsigned long F_MIN;
unsigned long F_MAX;

uint8_t magic_no = 02; // just for checking the initialization of eprom
uint16_t xpos, ypos;  //screen coordinates

/**************************************/
/* Interrupt service routine for      */
/* encoder frequency change           */
/**************************************/

ISR(PCINT2_vect) {
  unsigned char result = r.process();

  if (result == DIR_CW)
    vfo += radix;
  else if (result == DIR_CCW)
    vfo -= radix;
  changed_f = 1;
}


void setup()
{
  //Serial.begin(9600);  // for debugging
  uint16_t identifier = 0x154;  // identify display driver chip (known from mcufriend test progs)
  tft.begin(identifier);        // setup to use driver
  tft.setRotation(1); // LS
  Wire.begin();   // for si5351
  // welcome_screen();
  setup_vfo_screen(); // create main VFO screen
  display_msg(0, Ver); // displ at x=0

  if (EEPROM.read(eprom_base_addr) != magic_no)
    init_eprom();  // if blank eeprom or changed/new magic_no for reinit

  read_eprom(); // get infos of VFO A & B (and 1st memory channel only??)
  vfo = vfo_A;   // then display and use VFO A
  vfo_A_sel = 1;
  bfo = bfo_A;
  sideband = sb_A;


  //Initialise si5351
  si5351.set_correction(00); //. There is a calibration sketch in File/Examples/si5351Arduino-Jason; was 140
  //where you can determine the correction by using the serial monitor.

  //initialize the Si5351
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0); //If you're using a 27Mhz crystal, put in 27000000 instead of 0
  // 0 is the default crystal frequency of 25Mhz.

  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);

  // Set CLK0 to output the starting "vfo" frequency as set above by vfo = ?
  if (sideband == LSB)
    bfo = bfo_LSB;
  else
    bfo = bfo_USB;

  if (vfo < bfo)
    vfo_tx = bfo - vfo;
  else
    vfo_tx = vfo - bfo;

  si5351.set_freq(((vfo_tx + if_offset)* SI5351_FREQ_MULT), SI5351_CLK0);

  // Set CLK2 to output bfo frequency
  //  si5351.set_freq( bfo, 0, SI5351_CLK2);
  set_bfo();

  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA); //  this is 11dBm  // you can set this to 2MA, 4MA, 6MA or 8MA
  // si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_4MA); //be careful though - measure into 50ohms
  //si5351.drive_strength(SI5351_CLK2,SI5351_DRIVE_6MA); //

  pinMode(ENCODER_BTN, INPUT_PULLUP);     // step pushbutton setup
  pinMode(BandSelect, INPUT_PULLUP);     // band pushbutton setup
  pinMode(SideBandSelect, INPUT_PULLUP);     //sideband pushbutton setup
  // pinMode(TEST_OP, OUTPUT);  // test purpose only
  // test_op = HIGH;

  for (int i = 0; i <= MAX_BANDS; i++)   // all band control pins init & OFF
  {
    pinMode(band_cntrl[i], OUTPUT);
    digitalWrite(band_cntrl[i], LOW);
  }

  PCICR |= (1 << PCIE2);           // Enable pin change interrupt for the encoder
  //  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19); // uno
  PCMSK2 |= (1 << PCINT21) | (1 << PCINT22); // MEGA interrupt pins mapped to A14 A13
  sei();   // start interrupts

  display_frequency();  // Update the local display at power on
  display_vfo();
  display_band();       // with values
  display_step();
  display_sideband();
  display_mem();
  display_bfo();
} // end of setup() //

//--------------------------------------

void loop()  {

  //  test_op = ! test_op; // profiling - flip flop every time it enters loop() look on CRO
  //  digitalWrite(TEST_OP, test_op); // check loop timing while not touching any button & touching diff buttons

  // Update the display if the frequency changed
  if (changed_f)
  {
    display_frequency();
    set_band();
    display_band();
    display_sideband();

    if (vfo < bfo)
      vfo_tx = bfo - vfo;
    else
      vfo_tx = vfo - bfo;

    si5351.set_freq(((vfo_tx + if_offset)* SI5351_FREQ_MULT), SI5351_CLK0);
    changed_f = 0;
  }

  //-----------------------------------
  // External button controls
  //##### Band Select
  { if (digitalRead(BandSelect) == LOW )
    {
      for (debounce = 0; debounce < DebounceDelay; debounce++) {
      };
      bnd_count = bnd_count + 1;                        // Increment band selection

      display_band();
    }
  }

  //##### Step Size
  { if (digitalRead(ENCODER_BTN) == LOW )
    {
      for (debounce = 0; debounce < DebounceDelay; debounce++) {
      };
      step_index = step_index + 1;                        // Increment band selection
      if (step_index > 7)
      {
        step_index = 1;
      }
      display_step();
    }
  }

  //##### Sideband Select
  { if (digitalRead(SideBandSelect) == LOW )
    {
      for (debounce = 0; debounce < DebounceDelay; debounce++) {
      };
      sideband = sideband + 1;                        // Increment band selection
      if (sideband > 2 )
      {
        sideband = 1;
      }
      display_sideband();
    }
  }

  //=====================
  //$$$$ S Meter display
  int SENSOR = analogRead(66);
  //  tft.fillRect(3, 3, 10, 195, YELLOW);

  if (SENSOR < 700) {  // lower scale   upside down
    tft.fillRect(3, 3, 10, 195, YELLOW);
    tft.fillRect(3, 3, 10, (SENSOR / 10), MAGENTA);
  }
  if (SENSOR > 700) { // upper scale
    tft.fillRect(3, 3, 10, (SENSOR / 10), RED);  // $$$ to be chkd with var dc
    tft.fillRect(3, 3, 10, 150, MAGENTA);  // $$$ assuming 150 is S9 etc
  }

  //----------------------------------------------
  //$$$$  for Touch Screen input control
  tp = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  pinMode(XP, OUTPUT);
  pinMode(YM, OUTPUT);
  if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE)
  {
    xpos = map(tp.x, TS_LEFT, TS_RT, 0, tft.width());
    ypos = map(tp.y, TS_TOP, TS_BOT, 0, tft.height());

    if (ypos > 10 && ypos < 35)    // first row of buttons (orig 5,42)
    {
      // VFO Button:  cycle VFO A/B/M in sequence when VFO button is touched
      if (xpos > 25 && xpos < 90 ) // change VFO (orig 20,95)
      {
        if (vfo_M_sel)
        {
          vfo_M = vfo;
          vfo_A_sel = true;
          vfo_B_sel = false;
          vfo_M_sel = false;
          vfo = vfo_A;
          bfo = bfo_A;
          sideband = sb_A;
          if (sideband == USB)
            bfo_USB = bfo_A;
          else
            bfo_LSB = bfo_A;
        }
        else if (vfo_B_sel)
        {
          vfo_B = vfo; // save current value of vfo for use later
          if (!xch_M)
            read_ch();   // get data from memory channel
          else
            xch_M = 0;

          vfo_A_sel = false;
          vfo_B_sel = false;
          vfo_M_sel = true;  // select
          vfo = vfo_M;    // restore values
          bfo = bfo_M;
          sideband = sb_M;
          if (sideband == USB)
            bfo_USB = bfo_M;
          else
            bfo_LSB = bfo_M;
        }
        else if (vfo_A_sel)
        {
          vfo_A = vfo;
          vfo_A_sel = false;
          vfo_B_sel = true;
          vfo_M_sel = false;
          vfo = vfo_B;
          bfo = bfo_B;
          sideband = sb_B;
          if (sideband == USB)
            bfo_USB = bfo_B;
          else
            bfo_LSB = bfo_B;
        }
        set_vfo();
        display_vfo();
        save_frequency();
        display_bfo();
        set_bfo();
        display_sideband();
      }

      // MEM Ch change Button
      // Left half button decreases channel no
      else if (xpos > 115 && xpos < 175 )  // decrease channel ,(110,175)
      {
        memCh = memCh - 1;
        if (memCh <= 0)
          memCh = max_memory_ch;
        display_mem();
      }

      // right half buttton increases ch no
      else if (xpos > 176 && xpos < 240 ) // increase channel  (176,245)
      {
        memCh = memCh + 1;
        if (memCh > max_memory_ch)
          memCh = 1;
        display_mem();
      }
      return;
    }  // First row end

    //$$$$$ Freq Change Second Row  Button
    if (ypos > 50 && ypos < 80)   //(45,85)
    {
      if (xpos > 55 && xpos < 180 )  // Left half button decreases frq by step size (50,180)
      {
        vfo = vfo - radix;
        changed_f = 1;
      }

      else if (xpos > 185 && xpos < 305 )  // Right half button increases freq by step size (185,310)
      {
        vfo = vfo + radix;
        changed_f = 1;
      }
      save_frequency();   // added 7/7/17
      return;
    }  // Freq Button/ Second Row end

    //$$$$  Third Row  Band Change Button
    if (ypos > 118 && ypos < 150)     //(113,150)
    {
      if (xpos > 25 && xpos < 65 )       // Left half button decreases band(20,65)
      {
        old_band = bnd_count;
        bnd_count = bnd_count - 1;
        if (bnd_count < 0)
          bnd_count = 8;
        change_band();
        save_frequency();
      }

      else if (xpos > 67 && xpos < 110 )     // Right half button increases band (67,115)
      {
        old_band = bnd_count;
        bnd_count = bnd_count + 1;
        if (bnd_count > 8)
          bnd_count = 0;
        change_band();
        save_frequency();
      }


      // $$$$ Third Row Step Size change  Button
      //  left half of step button decreases step size
      else if (xpos > 120 && xpos < 175 )   //(114,175)
      {
        step_index = step_index - 1;
        if (step_index < 0)
          step_index = 6;
        display_step();
      }
      //  right half of step button increases step size
      else if (xpos > 177 && xpos < 232 )     // (177,238)
      {
        step_index = step_index + 1;
        if (step_index > 6)
          step_index = 0;
        display_step();
      }

      ///$$$$$ Third Row side band flip flop between LSB & USB   (others may be added if hardware permits)
      else if (xpos > 250 && xpos < 300 ) // (245,310)
      {
        if (sideband == LSB)
        {
          sideband = USB;
          bfo = bfo_USB;
        }
        else
        {
          sideband = LSB;
          bfo = bfo_LSB;
        }
        delay(50);
        display_sideband();
        display_bfo();
        save_frequency();
        set_vfo();
      }
      return;
    } // Third row end

    if (ypos > 158 && ypos < 180)   // (152,189)
    {
      //$$$$$ Fourth Row VFO < > Mem switch nothing saved on EEPROM unless SAVE button pressed
      if (xpos > 25 && xpos < 65 )   //left half VFO -> MEM (20,65)
        // currently selected VFO stored on currently selected mem ch (not in EEPROM which is by SAVE button)
      {
        if (vfo_A_sel)     // when vfo A is selected its content transferred to current memory
        {
          vfo_M = vfo_A;
          bfo_M = bfo_A;
        }
        else if (vfo_B_sel)   // B -> MemCh
        {
          vfo_M = vfo_B;
          bfo_M = bfo_B;
        }
        xch_M = 1;
        display_frequency2();
      }
      else if (xpos > 66 && xpos < 104 ) //right half VFO <- MEM    (66,109)
      {
        if ( vfo_A_sel)   // when vfo A is working Mem goes to A and changes vfo freq
        {
          vfo_A = vfo_M;
          vfo = vfo_A;
          bfo_A = bfo_M;
          bfo = bfo_A;
        }
        else if ( vfo_B_sel)  // when vfo B is working Mem goes to B and changes vfo freq
        {
          vfo_B = vfo_M;
          vfo = vfo_B;
          bfo_B = bfo_M;
          bfo = bfo_B;
        }
        changed_f = 1;
      }

      // $$$$$ Fourth Row bfo freq adjust

      // left half button decreases bfo freq
      else if (xpos > 116 && xpos < 175 ) // decrease freq
      {
        bfo = bfo - radix;
        set_bfo();
        display_bfo();
        save_frequency();
      }
      // right half button increases bfo freq
      else if (xpos > 178 && xpos < 230 ) // increase freq  (175,235)
      {
        bfo = bfo + radix;
        set_bfo();
        display_bfo();
        save_frequency();
      }

      // Fourth Row SAVE Button
      else if (xpos > 250 && xpos < 305 )  // Save "current" Vfo/Mem on eeprom  ?? may shift below A/B/M display 2nd row or use that itself for saving (245,310)
      {
        display_msg(60, "Saving..");
        if (vfo_M_sel)
          write_ch();
        else if (vfo_A_sel)
          write_vfo_A();
        else
          write_vfo_B();
        display_msg(60, "        ");
      }
    } // Fourth Row end
  }
}    // end of loop

