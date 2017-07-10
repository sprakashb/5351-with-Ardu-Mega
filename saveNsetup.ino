void update_display()
{
  save_frequency();
  display_frequency();
  set_band();
  display_band();
  display_sideband();
}

void set_vfo()
{
  if (vfo < bfo)
    vfo_tx = bfo - vfo;
  else
    vfo_tx = vfo - bfo;

  si5351.set_freq(((vfo_tx + if_offset)* SI5351_FREQ_MULT), SI5351_CLK0);
}


void save_frequency()    // for temporarily saving in variables not in EEPROM
{
  if (vfo_M_sel)
  {
    vfo_M = vfo;
    bfo_M = bfo;
    sb_M = sideband;
  }
  else if (vfo_A_sel)
  {
    vfo_A = vfo;
    bfo_A = bfo;
    sb_A = sideband;
  }
  else
  {
    vfo_B = vfo;
    bfo_B = bfo;
    sb_B = sideband;
  }
}

void set_band()       // from frequecy determine band and activate corresponding relay
{
  old_band = bnd_count;
  for (int i = MAX_BANDS; i >= 0; i--)
  {
    if ((vfo >= F_MIN_T[i]) && (vfo <= F_MAX_T[i]))
    {
      bnd_count = i ;
      break;
    }
  }
  digitalWrite(band_cntrl[old_band], LOW);   // deactivate old band relay
  digitalWrite(band_cntrl[bnd_count], HIGH); // activate new selected band
}

void set_bfo()
{
  si5351.set_freq((bfo * SI5351_FREQ_MULT), SI5351_CLK2);
}


void setup_vfo_screen() // sets up main screen for VFO etc display
{
  tft.fillScreen(BLUE); // setup blank screen LIGHTGREY
 // tft.fillRect(0, 0, 320, 121, BLUE); // top segment
  tft.drawRect(0, 0, 320, 240, WHITE);  // outer full border
  tft.setTextSize(2);
  tft.setTextColor(WHITE);  //

  tft.drawRect(0, 0, 16, 200, RED); // vert S meter
  tft.fillRect(3, 3, 10, 195, YELLOW);
  tft.setTextSize(2);
  tft.setTextColor(RED);
  tft.setCursor(6, 205);
  tft.println("S");

  tft.drawRoundRect(20, 5, 75, 37, 14, RED);  // VFO A/B box outline
  tft.fillRoundRect(22, 7, 71, 33, 10, GREEN);   //VFO A/B box
  tft.setCursor(30, 12);
  tft.setTextSize(3);
  tft.setTextColor(RED);
  tft.print("VFO");

  tft.drawRoundRect(110, 5, 135, 37, 14, RED);  // Mem box outline
  tft.fillRoundRect(112, 7, 131, 33, 10, GREY);   //Mem box
  tft.setCursor(125, 12);
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.print("MEM ");
  display_mem();

  tft.drawRoundRect(260, 5, 50, 37, 14, RED);  // TxRx box outline
  tft.fillRoundRect(262, 7, 45, 33, 10, GREEN);   //TxRx box
  tft.setCursor(268, 12);
  tft.setTextSize(3);
  tft.setTextColor(BLUE);
  tft.print("Rx");

  tft.drawRoundRect(50, 45, 264, 40, 14, WHITE);  // freq box outline
  // tft.fillRoundRect(52, 47, 260, 36, 10, ORANGE);   //freq box

  tft.fillRoundRect(20, 113, 89, 37, 14, WHITE); //band button outline
  tft.fillRoundRect(22, 115, 85, 33, 10, GREY); //band

  tft.fillRoundRect(114, 113, 124, 37, 14, WHITE); // step button outline
  tft.fillRoundRect(116, 115, 120, 33, 10, GREY); //step

  tft.fillRoundRect(243, 113, 69, 37, 14, WHITE); // sideband button outline
  tft.fillRoundRect(245, 115, 65, 33, 10, GREEN); //sideband

  tft.drawRoundRect(20, 152, 89, 37, 14, RED); //  VFO <> MEM  button outline
  tft.fillRoundRect(22, 154, 85, 33, 10, GREY); //  V/M
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(30, 164);
  tft.print("V>  <M");

  tft.drawRoundRect(114, 152, 124, 37, 14, RED); //bfo button outline
  tft.fillRoundRect(116, 154, 120, 33, 10, GREY); //bfo
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(130, 164);
  tft.print(bfo);

  tft.drawRoundRect(243, 152, 69, 37, 14, MAGENTA); // Save button outline
  tft.fillRoundRect(245, 154, 65, 33, 10, RED); //Save
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(253, 164);
  tft.print("SAVE");

// 5th row of buttons
  tft.drawRoundRect(20, 192, 70, 28, 14, GREEN); // F1 button outline
  tft.fillRoundRect(22, 194, 66, 24, 10, ORANGE); //F1
  tft.setTextSize(2);
  tft.setTextColor(BLUE);
  tft.setCursor(47, 198);
  tft.print("F1");

 tft.drawRoundRect(95, 192, 70, 28, 14, GREEN); // F2 button outline
  tft.fillRoundRect(97, 194, 66, 24, 10, ORANGE); //F2
  tft.setTextSize(2);
  tft.setTextColor(BLUE);
  tft.setCursor(115, 198);
  tft.print("F2");

 tft.drawRoundRect(165, 192, 70, 28, 14, GREEN); // F1 button outline
  tft.fillRoundRect(167, 194, 66, 24, 10, ORANGE); //F1
  tft.setTextSize(2);
  tft.setTextColor(BLUE);
  tft.setCursor(190, 198);
  tft.print("F3");

 tft.drawRoundRect(240, 192, 70, 28, 14, GREEN); // F1 button outline
  tft.fillRoundRect(242, 194, 66, 24, 10, ORANGE); //F1
  tft.setTextSize(2);
  tft.setTextColor(GREEN);
  tft.setCursor(265, 198);
  tft.print("F4");

  tft.fillRect(0, 220, 320, 21, BLUE);  // bot strip
  tft.drawRect(0, 220, 320, 21, WHITE);  // surrounding RECT
  tft.setTextColor(WHITE, BLUE);

}  // end of setup_vfo_screen()
//---------------------------


/////// $$$ EEPROM related
void init_eprom()      // write some info on EEPROM when initially loaded or when magic number changes
{
  EEPROM.write(eprom_base_addr, magic_no);  // check byte may be same as ver no at 0 address
  display_msg(60, "Init EEPROM");
  ch_info = {vfo_A, bfo_LSB, LSB};
  address = eprom_base_addr + 1 + sizeof(ch_info) * 1;  // initial infos for VFO A
  EEPROM_writeAnything(address, ch_info);

  //  ch_info={vfo_B, bfo_USB, USB};  //or
  ch_info.s_vfo = vfo_B;   // initial values of VFO B
  ch_info.s_bfo = bfo_USB;
  ch_info.s_sb = 2 ;
  address = eprom_base_addr + 1 + sizeof(ch_info) * 2;  // initial infos for VFO B
  EEPROM_writeAnything(address, ch_info);

  // Now store all 13 channels

  for (int i = 1; i <= MAX_BANDS; i++)  // starting from 1st entry in table of freq
  {
    ch_info.s_vfo = VFO_T[i];
    if (VFO_T[i] < 10000000)
    {
      ch_info.s_bfo = bfo_LSB;
      ch_info.s_sb = LSB;
    }
    else
    {
      ch_info.s_bfo = bfo_USB;
      ch_info.s_sb = USB;
    }
    address = eprom_base_addr + 1 + sizeof(ch_info) * (i + 2); // first byte for magic no and first 2 infos for VFO A & B
    EEPROM_writeAnything(address, ch_info);
  }

  vfo = 7100000;
  for (int i = MAX_BANDS+1; i <= 24; i++)  // starting from 
  {
    ch_info.s_vfo = vfo;
    ch_info.s_bfo = bfo_LSB;
    ch_info.s_sb = LSB;
    address = eprom_base_addr + 1 + sizeof(ch_info) * (i + 2); // first byte for magic no and first 2 infos for VFO A & B
    EEPROM_writeAnything(address, ch_info);
  }
  vfo = 14000000;
  for (int i = 25; i <= 50; i++)  // starting from 160m
  {
    ch_info.s_vfo = vfo;
    ch_info.s_bfo = bfo_USB;
    ch_info.s_sb = USB;
    address = eprom_base_addr + 1 + sizeof(ch_info) * (i + 2); // first byte for magic no and first 2 infos for VFO A & B
    EEPROM_writeAnything(address, ch_info);
  }
  vfo = 14200000;
  for (int i = 51; i <= max_memory_ch; i++)  // starting from 160m
  {
    ch_info.s_vfo = vfo;
    ch_info.s_bfo = bfo_USB;
    ch_info.s_sb = USB;
    address = eprom_base_addr + 1 + sizeof(ch_info) * (i + 2); // first byte for magic no and first 2 infos for VFO A & B
    EEPROM_writeAnything(address, ch_info);
  }
  display_msg(60, "           ");
}

void read_eprom()     // should be called at powerup to retrieve stored values for vfos A and B
{
  display_msg(60, "Read EEPROM");

  address = eprom_base_addr + 1 + sizeof(ch_info) * 1;  // first infos for VFO A
  EEPROM_readAnything(address, ch_info);
  vfo_A = ch_info.s_vfo ;   // initial values of VFO A
  bfo_A = ch_info.s_bfo;
  sb_A = ch_info.s_sb;

  address = eprom_base_addr + 1 + sizeof(ch_info) * 2;  // second infos for VFO B
  EEPROM_readAnything(address, ch_info);
  vfo_B = ch_info.s_vfo ;   // initial values of VFO B
  bfo_B = ch_info.s_bfo;
  sb_B = ch_info.s_sb;

  memCh = 1;
  read_ch();   // for 1st mem channel also
  display_msg(60, "           ");
}

void read_ch()    // read channel info from eeprom when ever mem ch no changed
{
  address = eprom_base_addr + 1 + sizeof(ch_info) * (memCh + 2); // info for mem channel displayed
  EEPROM_readAnything(address, ch_info);
  vfo_M = ch_info.s_vfo ;
  bfo_M = ch_info.s_bfo ;
  sb_M = ch_info.s_sb;
}

void write_ch()   // write memory channel into eeprom
{
  ch_info = {vfo_M, bfo_M, sb_M};
  address = eprom_base_addr + 1 + sizeof(ch_info) * (memCh + 2); // initial infos for VFO A
  EEPROM_writeAnything(address, ch_info);
}

void write_vfo_A()
{
  ch_info = {vfo_A, bfo_A, sb_A};
  address = eprom_base_addr + 1 + sizeof(ch_info) * 1;  // initial infos for VFO A
  EEPROM_writeAnything(address, ch_info);

}
void write_vfo_B()
{
  ch_info = {vfo_B, bfo_B, sb_B};
  address = eprom_base_addr + 1 + sizeof(ch_info) * 2;  // initial infos for VFO B
  EEPROM_writeAnything(address, ch_info);

}

