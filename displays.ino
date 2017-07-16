void display_mem()
{
  tft.setCursor(185, 12);
  tft.setTextSize(3);
  tft.setTextColor(GREEN, GREY);
  if (memCh < 10)
    tft.print("0");
  tft.print(memCh);
  if (memCh < 100)
    tft.print(" ");
  if (!xch_M)
    read_ch();
  else
    xch_M = 0;

  if (vfo_M_sel)
  {
    vfo = vfo_M;
    bfo = bfo_M;
    display_bfo();
    set_bfo();
    sideband = sb_M;
    display_sideband();
    display_frequency();
  }
  else
    display_frequency2();
}

void display_vfo()
{
  tft.setCursor(25, 50);
  tft.setTextSize(4);
  tft.setTextColor(WHITE, BLUE);
  if (vfo_M_sel)
    tft.print("M");  // Mem   ....
  else if (vfo_A_sel)
    tft.print("A");  // VFO A or B  ....
  else
    tft.print("B");  // VFO A or B  ....

  display_frequency2(); // 2nd line of display only when vfos changed
  set_band();  // display band according to frequency displayed
  display_band();
}


void display_frequency()
{
  tft.setTextSize(4);
  tft.setTextColor(WHITE, BLUE);
  tft.setCursor(70, 50);
  if (vfo < 10000000)
    tft.print(" ");
  tft.print(vfo / 1000.0, 3);
}

void display_frequency2()
{
  //other 2 vfo's displayed below
  tft.setTextSize(2);
  tft.setTextColor(WHITE, BLUE);
  tft.setCursor(25, 93);

  if (vfo_A_sel)
  {
    tft.print("B ");
    tft.print(vfo_B / 1000.0, 3);
    tft.setCursor(170, 93);
    tft.print("M ");
    tft.print(vfo_M / 1000.0, 3);
    tft.print(" "); // takes care of previous leftover digit
  }
  if (vfo_B_sel)
  { tft.print("A ");
    tft.print(vfo_A / 1000.0, 3);
    tft.setCursor(170, 93);
    tft.print("M ");
    tft.print(vfo_M / 1000.0, 3);
    tft.print(" ");
  }
  if (vfo_M_sel)
  {
    tft.print("A ");
    tft.print(vfo_A / 1000.0, 3);
    tft.setCursor(170, 93);
    tft.print("B ");
    tft.print(vfo_B / 1000.0, 3);
    tft.print(" ");
  }
} // end of display_frequency2()

void set_band()       // from frequecy determine band and activate corresponding relay
{
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

void display_band()
{
  tft.setCursor(22, 125);
  tft.setTextSize(2);
  tft.setTextColor(WHITE, GREY);
  changed_f = 1;
  tft.print(B_NAME_T[bnd_count]);
}  // end of Display-band()

void change_band() {
  // tft.setCursor(22, 125);
  // tft.setTextSize(2);
  // tft.setTextColor(WHITE, GREY);
  display_band();
  F_MIN = F_MIN_T[bnd_count];
  F_MAX = F_MAX_T[bnd_count];
  vfo = VFO_T[bnd_count];
  //  set_band();
  changed_f = 1;
}  // end of change_band()

// Displays the frequency change step
void display_step()
{ tft.setCursor(117, 125);
  tft.setTextSize(2);
  tft.setTextColor(WHITE, GREY);
  tft.print(step_sz_txt[step_index]);
  radix = step_sz[step_index];
}

void display_sideband() {
  tft.setCursor(261, 125);
  tft.setTextSize(2);
  tft.setTextColor(RED, GREEN);
  if (sideband == LSB)
  {
    tft.print("LSB");
  }
  else if (sideband == USB)
  {
    tft.print("USB");
  }
}

void display_bfo()
{
  tft.setTextSize(2);
  tft.setTextColor(WHITE, GREY);
  tft.setCursor(130, 164);  // also in setup_vfo_screen
  tft.print(bfo);
  if (bfo < 10000000)
    tft.print(" ");
}

void display_msg(int xposn, String msg)
{ tft.setTextSize(2); // may setp some soft buttons here
  tft.setCursor(xposn, 223);
  tft.setTextColor(WHITE, BLUE);
  tft.println(msg);
}
