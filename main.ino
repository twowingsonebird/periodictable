// Include Libraries
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "Adafruit_PWMServoDriver.h"
#include <FastLED.h>
#include <LiquidCrystal_I2C.h>

#define LED_TYPE WS2811   // This is the type of LED in the strip
#define LED_PIN 3         // This is the pin the LED strip is connnected to
#define COLOR_ORDER GRB   // This is the order of the colors in the LED strip, most are GRB (Green, Red, Blue)
#define NUM_LEDS 18       // This is the total number of LEDs in the strip
#define BRIGHTNESS 10     // This is the brightness (0-255)
#define UNKNOWN_VALUE -1  // This is a placeholder value for undiscovered values, we put -1 because it's a number we'll never use

LiquidCrystal_I2C lcd(0x27, 20, 4);  // Create an LCD object with I2C communication, the first argument is the I2C address, the second is the number of columns, and the third is the number of rows

CRGB leds[NUM_LEDS];  // Create the LED object

// Define the upper & lower bounds for each trend
const float mmUpper = 294.0;  // Oganesson
const float mmLower = 1.008;  // Hydrogen

const float mpUpper = 3915.0;  // Carbon
const float mpLower = 14.01;   // Helium

const float bpUpper = 5869.0;  // Rhenium
const float bpLower = 4.22;    // Helium

const float enUpper = 23.98;  // Fluorine
const float enLower = 0.7;    // Francium

const float ieUpper = 2372.3;  // Helium
const float ieLower = 375.7;   // Cesium

const float arUpper = 298.0;  // Cesium
const float arLower = 31.0;   // Helium

const float oneSide = (200.0 / 9);  // This is how many steps to take to rotate one side of the prism

float stepsMade = 0.0;  // This float keeps track of the total number of steps made

const int numRows = 9;     // The number of rows (9 sides on the prism so 9)
const int numTrends = 6;   // The number of periodic trends (molar mass, melting point, etc.)
const int homeButton = 2;  // Define the pin for the home button
// Define button pins for each trend
const int mmButton = 34;
const int mpButton = 35;
const int bpButton = 36;
const int enButton = 37;
const int ieButton = 38;
const int arButton = 39;
const int stateButton = 40;
const int tempPot = A0;             // The pin for the temperature potentiometer
const int stepsPerRevolution = 200;  // The number of steps in a full rotation (200 for this motor)

const int rowPins[numRows] = { 3, 4 };  // Create a constant integer array with the size of how many rows we have, containing the pin numbers for the button to each row

int currentRow = 1;   // Create an integer which will tell us what row we're on, starting at row 1
int stepperPort = 1;  // This is the port the stepper is connected to, M1/M2 would be port 1 and M3/M4 would be port 2 (on the Adafruit Motor Shield V2)

struct Element {  // Create  a struct named const Element with the periodic trends and the name of each const Element as its members
  char name[3];
  float molarMass;
  float meltingPoint;
  float boilingPoint;
  float electroNegativity;
  float ionizationEnergy;
  float atomicRadius;
};

// Create an const Element struct for each const Element with their respective values
const Element H = { "H", 1.008, 14.01, 20.28, 2.20, 1312.0, 53 };
const Element He = { "He", 4.002602, UNKNOWN_VALUE, 4.22, UNKNOWN_VALUE, 2372.3, 31 };
const Element Li = { "Li", 6.94, 453.69, 1615., 0.98, 520.2, 167 };
const Element Be = { "Be", 9.0121831, 1560., 2743., 1.57, 899.5, 112 };
const Element B = { "B", 10.81, 2348., 4273., 2.04, 800.6, 87 };
const Element C = { "C", 12.011, 3915., 3915., 2.55, 1086.5, 67 };
const Element N = { "N", 14.007, 63.05, 77.36, 3.04, 1402.3, 56 };
const Element O = { "O", 15.999, 54.8, 90.2, 3.44, 1313.9, 48 };
const Element F = { "F", 18.998403162, 53.5, 85.03, 3.98, 1681.0, 42 };
const Element Ne = { "Ne", 20.1797, 24.56, 27.07, UNKNOWN_VALUE, 2080.7, 38 };
const Element Na = { "Na", 22.98976928, 370.87, 1156., 0.93, 495.8, 190 };
const Element Mg = { "Mg", 24.305, 923., 1363., 1.31, 737.7, 145 };
const Element Al = { "Al", 26.9815384, 933.47, 2792., 1.61, 577.5, 118 };
const Element Si = { "Si", 28.085, 1687., 3173., 1.90, 786.5, 111 };
const Element P = { "P", 30.973761998, 317.3, 553.6, 2.19, 1011.8, 98 };
const Element S = { "S", 32.06, 388.36, 717.87, 2.58, 999.6, 88 };
const Element Cl = { "Cl", 35.45, 171.6, 239.11, 3.16, 1251.2, 79 };
const Element Ar = { "Ar", 39.948, 83.8, 87.3, UNKNOWN_VALUE, 1520.6, 71 };
const Element K = { "K", 39.0983, 336.53, 1032., 0.82, 418.8, 243 };
const Element Ca = { "Ca", 40.078, 1115., 1757., 1.0, 589.8, 194 };
const Element Sc = { "Sc", 44.955907, 1814., 3103., 1.36, 633.1, 184 };
const Element Ti = { "Ti", 47.867, 1941., 3560., 1.54, 658.8, 176 };
const Element V = { "V", 50.9415, 2183., 3680., 1.63, 650.9, 171 };
const Element Cr = { "Cr", 51.9961, 2180., 2944., 1.66, 652.9, 166 };
const Element Mn = { "Mn", 54.938043, 1519., 2334., 1.55, 717.3, 161 };
const Element Fe = { "Fe", 55.845, 1811., 3134., 1.83, 762.5, 156 };
const Element Co = { "Co", 58.933194, 1768., 3.2e3, 1.88, 760.4, 152 };
const Element Ni = { "Ni", 58.6934, 1728., 3186., 1.91, 737.1, 149 };
const Element Cu = { "Cu", 63.546, 1357.77, 2835., 1.90, 745.5, 145 };
const Element Zn = { "Zn", 65.38, 692.68, 1180., 1.65, 906.4, 142 };
const Element Ga = { "Ga", 69.723, 302.91, 2477., 1.81, 578.8, 136 };
const Element Ge = { "Ge", 72.63, 1211.4, 3093., 2.01, 762, 125 };
const Element As = { "As", 74.921595, 1090., 887., 2.18, 947.0, 114 };
const Element Se = { "Se", 78.971, 494., 958., 2.55, 941.0, 103 };
const Element Br = { "Br", 79.904, 265.8, 332., 2.96, 1139.9, 94 };
const Element Kr = { "Kr", 83.798, 115.79, 119.93, 3.0, 1350.8, 88 };
const Element Rb = { "Rb", 85.4678, 312.46, 961., 0.82, 403.0, 265 };
const Element Sr = { "Sr", 87.62, 1050., 1655., 0.95, 549.5, 219 };
const Element Y = { "Y", 88.905838, 1799., 3618., 1.22, 600, 212 };
const Element Zr = { "Zr", 91.224, 2128., 4682., 1.33, 640.1, 206 };
const Element Nb = { "Nb", 92.90637, 2750., 5017., 1.6, 652.1, 198 };
const Element Mo = { "Mo", 95.95, 2896., 4912., 2.16, 684.3, 190 };
const Element Tc = { "Tc", 98, 2430., 4538., 1.9, 702, 183 };
const Element Ru = { "Ru", 101.07, 2607., 4423., 2.2, 710.2, 178 };
const Element Rh = { "Rh", 102.90549, 2237., 3968., 2.28, 719.7, 173 };
const Element Pd = { "Pd", 106.42, 1828.05, 3236., 2.20, 804.4, 169 };
const Element Ag = { "Ag", 107.8682, 1234.93, 2435., 1.93, 731.0, 165 };
const Element Cd = { "Cd", 112.414, 594.22, 1040., 1.69, 867.8, 161 };
const Element In = { "In", 114.818, 429.75, 2345., 1.78, 558.3, 156 };
const Element Sn = { "Sn", 118.710, 505.08, 2875., 1.96, 708.6, 145 };
const Element Sb = { "Sb", 121.760, 903.78, 1860., 2.05, 834, 133 };
const Element Te = { "Te", 127.60, 722.66, 1261., 2.1, 869.3, 123 };
const Element I = { "I", 126.90447, 386.85, 457.4, 2.66, 1008.4, 115 };
const Element Xe = { "Xe", 131.293, 161.3, 165.1, 2.6, 1170.4, 108 };
const Element Cs = { "Cs", 132.90545196, 301.59, 944., 0.79, 375.7, 298 };
const Element Ba = { "Ba", 137.327, 1.0e3, 2143., 0.89, 502.9, 253 };
const Element La = { "La", 138.90547, 1193., 3737., 1.10, 538.1, UNKNOWN_VALUE };
const Element Ce = { "Ce", 140.116, 1071., 3633., 1.12, 534.4, UNKNOWN_VALUE };
const Element Pr = { "Pr", 140.90766, 1204., 3563., 1.13, 527, 247 };
const Element Nd = { "Nd", 144.242, 1294., 3373., 1.14, 533.1, 206 };
const Element Pm = { "Pm", 145, 1373., 3273., UNKNOWN_VALUE, 540, 205 };
const Element Sm = { "Sm", 150.36, 1345., 2076., 1.17, 544.5, 238 };
const Element Eu = { "Eu", 151.964, 1095., 1.8e3, UNKNOWN_VALUE, 547.1, 231 };
const Element Gd = { "Gd", 157.25, 1586., 3523., 1.20, 593.4, 233 };
const Element Tb = { "Tb", 158.925354, 1629., 3503., UNKNOWN_VALUE, 565.8, 225 };
const Element Dy = { "Dy", 162.500, 1685., 2840., 1.22, 573.0, 228 };
const Element Ho = { "Ho", 164.930329, 1747., 2973., 1.23, 581.0, 226 };
const Element Er = { "Er", 167.259, 1770., 3141., 1.24, 589.3, 226 };
const Element Tm = { "Tm", 168.934219, 1818., 2223., 1.25, 596.7, 222 };
const Element Yb = { "Yb", 173.045, 1092., 1469., UNKNOWN_VALUE, 603.4, 222 };
const Element Lu = { "Lu", 174.9668, 1936., 3675., 1.27, 523.5, 217 };
const Element Hf = { "Hf", 178.486, 2506., 4876., 1.3, 658.5, 208 };
const Element Ta = { "Ta", 180.94788, 3290., 5731., 1.5, 761, 200 };
const Element W = { "W", 183.84, 3695., 5828., 2.36, 770, 193 };
const Element Re = { "Re", 186.207, 3459., 5869., 1.9, 760, 188 };
const Element Os = { "Os", 190.23, 3306., 5285., 2.2, 840, 185 };
const Element Ir = { "Ir", 192.217, 2739., 4701., 2.20, 880, 180 };
const Element Pt = { "Pt", 195.084, 2041.4, 4098., 2.28, 870, 177 };
const Element Au = { "Au", 196.966570, 1337.33, 3129., 2.54, 890.1, 174 };
const Element Hg = { "Hg", 200.59, 234.32, 629.88, 2.0, 1007.1, 171 };
const Element Tl = { "Tl", 204.38, 577., 1746., 1.62, 589.4, 156 };
const Element Pb = { "Pb", 207.2, 600.61, 2022., 2.33, 715.6, 154 };
const Element Bi = { "Bi", 208.98040, 544.4, 1837., 2.02, 703, 143 };
const Element Po = { "Po", 209, 528., 1235., 2.0, 812.1, 135 };
const Element At = { "At", 210, 575., 623., 2.2, 890, 127 };
const Element Rn = { "Rn", 222, 202., 211.3, UNKNOWN_VALUE, 1037, 120 };
const Element Fr = { "Fr", 223, 294., 923., 0.7, 380, UNKNOWN_VALUE };
const Element Ra = { "Ra", 226, 973., 2010., 0.9, 509.3, UNKNOWN_VALUE };
const Element Ac = { "Ac", 227, 1323., 3473., 1.1, 499, UNKNOWN_VALUE };
const Element Th = { "Th", 232.0377, 2023., 5093., 1.3, 587, UNKNOWN_VALUE };
const Element Pa = { "Pa", 231.03588, 1845., 4273., 1.5, 568, UNKNOWN_VALUE };
const Element U = { "U", 238.02891, 1408., 4.2e3, 1.38, 597.6, UNKNOWN_VALUE };
const Element Np = { "Np", 237, 917., 4273., 1.36, 604.5, UNKNOWN_VALUE };
const Element Pu = { "Pu", 244, 913., 3503., 1.28, 584.7, UNKNOWN_VALUE };
const Element Am = { "Am", 243, 1449., 2284., 1.3, 578, UNKNOWN_VALUE };
const Element Cm = { "Cm", 247, 1618., 3383., 1.3, 581, UNKNOWN_VALUE };
const Element Bk = { "Bk", 247, 1323., UNKNOWN_VALUE, 1.3, 601, UNKNOWN_VALUE };
const Element Cf = { "Cf", 251, 1173., UNKNOWN_VALUE, 1.3, 608, UNKNOWN_VALUE };
const Element Es = { "Es", 252, 1133., UNKNOWN_VALUE, 1.3, 619, UNKNOWN_VALUE };
const Element Fm = { "Fm", 257, 1.8e3, UNKNOWN_VALUE, 1.3, 627, UNKNOWN_VALUE };
const Element Md = { "Md", 258, 1.1e3, UNKNOWN_VALUE, 1.3, 635, UNKNOWN_VALUE };
const Element No = { "No", 259, 1.1e3, UNKNOWN_VALUE, 1.3, 642, UNKNOWN_VALUE };
const Element Lr = { "Lr", 266, 1.9e3, UNKNOWN_VALUE, UNKNOWN_VALUE, 470, UNKNOWN_VALUE };
const Element Rf = { "Rf", 267, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, 580, UNKNOWN_VALUE };
const Element Db = { "Db", 268, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE };
const Element Sg = { "Sg", 269, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE };
const Element Bh = { "Bh", 270, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE };
const Element Hs = { "Hs", 270, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE };
const Element Mt = { "Mt", 278, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE };
const Element Ds = { "Ds", 281, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE };
const Element Rg = { "Rg", 282, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE };
const Element Cn = { "Cn", 285, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE };
const Element Nh = { "Nh", 286, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE };
const Element Fl = { "Fl", 289, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE };
const Element Mc = { "Mc", 290, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE };
const Element Lv = { "Lv", 293, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE };
const Element Ts = { "Ts", 294, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE };
const Element Og = { "Og", 294, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE, UNKNOWN_VALUE };
const Element blk = { "blank", 0, 0, 0, 0, 0, 0 };

const Element periodicTable[9][18] = {  // Create a 2D array to hold all these structs, each row and column is equal to the real periodic table
  { H, He, blk, blk, blk, blk, blk, blk, blk, blk, blk, blk, blk, blk, blk, blk, blk, blk },
  { Li, Be, B, C, N, O, F, Ne, blk, blk, blk, blk, blk, blk, blk, blk, blk, blk },
  { Na, Mg, Al, Si, P, S, Cl, Ar, blk, blk, blk, blk, blk, blk, blk, blk, blk, blk },
  { K, Ca, Sc, Ti, V, Cr, Mn, Fe, Co, Ni, Cu, Zn, Ga, Ge, As, Se, Br, Kr },
  { Rb, Sr, Y, Zr, Nb, Mo, Tc, Ru, Rh, Pd, Ag, Cd, In, Sn, Sb, Te, I, Xe },
  { Cs, Ba, La, Hf, Ta, W, Re, Os, Ir, Pt, Au, Hg, Tl, Pb, Bi, Po, At, Rn },
  { Fr, Ra, Ac, Rf, Db, Sg, Bh, Hs, Mt, Ds, Rg, Cn, Nh, Fl, Mc, Lv, Ts, Og },
  { blk, blk, Ce, Pr, Nd, Pm, Sm, Eu, Gd, Tb, Dy, Ho, Er, Tm, Yb, Lu, blk, blk },
  { blk, blk, Th, Pa, U, Np, Pu, Am, Cm, Bk, Cf, Es, Fm, Md, No, Lr, blk, blk }
};

CRGB trendColors[6][2] = {
  // This array holds CRGBs (an object representing RGB in color space, basically a color the LED can take defined by R, G, B) equivalent to the colors of the trends
  { CRGB::DarkGray, CRGB::Red },          // Molar Mass (Black to Red)
  { CRGB::Blue, CRGB::Goldenrod },        // Electronegativity (Blue to Yellow)
  { CRGB::Purple, CRGB::DarkGoldenrod },  // Ionization Energy (Purple to Gold)
  { CRGB::CornflowerBlue, CRGB::Red },    // Melting Point (Blue to Red)
  { CRGB::CornflowerBlue, CRGB::Red },    // Boiling Point (Blue to Red)
  { CRGB::DarkGray, CRGB::DarkCyan }      // Atomic Radius (Black to DarkCyan)
};

enum periodicTrends {  // This an enumerator, basically named values that represent constant integerals like 0,1,2 which we'll use to access the array easily
  molarMass,
  weight,
  electroNegativity,
  ionizationEnergy,
  meltingPoint,
  boilingPoint,
  atomicRadius,
};

periodicTrends trendWanted;  // Create a periodicTrends enumerator variable that will hold what trend we want

Adafruit_MotorShield AFMS = Adafruit_MotorShield();  // Initialize the motor shield
Adafruit_StepperMotor *myMotor = AFMS.getStepper(stepsPerRevolution, stepperPort);

CRGB interpolateColors(const CRGB &color1, const CRGB &color2, float value) {  // This function "interpolates" two colors, sort of like a gradient according to the value, which is a given ratio
  uint8_t r = lerp8by8(color1.r, color2.r, value);                             // lerp8by8 is a function that Linearly intERPolates two 8 bit values, here being our RGB values
  uint8_t g = lerp8by8(color1.g, color2.g, value);
  uint8_t b = lerp8by8(color1.b, color2.b, value);
  return CRGB(r, g, b);  // Return the interpolated color
}

CRGB lightUp(int row, int column, enum periodicTrends trend) {
  float trendValue = 0.0;  // Initialize a float trendValue to 0
  float lowerBound;        // Declare the floats for lower and higher bounds
  float higherBound;

  // Get the "low" and "high" (min/max) colors from the earlier array of colors
  CRGB lowColor = trendColors[trend][0];
  CRGB highColor = trendColors[trend][1];
  // This switch case assigns the trend value, as well as lower and higher bound based on the trend given
  switch (trend) {
    case molarMass:
      trendValue = periodicTable[row][column].molarMass;
      lowerBound = mmLower;
      higherBound = mmUpper;
      break;
    case electroNegativity:
      trendValue = periodicTable[row][column].electroNegativity;
      lowerBound = enLower;
      higherBound = enUpper;
      break;
    case ionizationEnergy:
      trendValue = periodicTable[row][column].ionizationEnergy;
      lowerBound = ieLower;
      higherBound = ieUpper;
      break;
    case meltingPoint:
      trendValue = periodicTable[row][column].meltingPoint;
      lowerBound = mpLower;
      higherBound = mpUpper;
      break;
    case boilingPoint:
      trendValue = periodicTable[row][column].boilingPoint;
      lowerBound = bpLower;
      higherBound = bpUpper;
      break;
    case atomicRadius:
      trendValue = periodicTable[row][column].atomicRadius;
      lowerBound = arLower;
      higherBound = arUpper;
      break;
    default:
      trendValue = 0.0;  // Default value if trend is not recognized
      lowerBound = 0;    // Default lower bound
      higherBound = 0;   // Default higher bound
      Serial.println(F("Trend not recognized!"));
      break;
  }
  if (trendValue == UNKNOWN_VALUE) {  // If the trend is one of the unknowns
    trendValue = 0.0;                 // Then just set it to 0 (no lights)
  }
  float ratio = map(trendValue, lowerBound, higherBound, 0.0, 1.0);  // The ratio is the trend value between the lower and higher bound, mapped to 0-1
  CRGB color = interpolateColors(lowColor, highColor, ratio);        // Interpolate the color based on the ratio
  return color;                                                      // Return it
}

void changeRow(int wantedRow) {                         // This function changes rows
  float stepsNeeded = wantedRow * oneSide - stepsMade;  // The steps needed to make from the current position is the wantedRow times the number of steps for one side, minus the steps already made
  if (stepsNeeded > 0) {                                // If it's a positive number (go forward)
    myMotor->step(stepsNeeded, FORWARD, DOUBLE);        // Then take that many steps forward
    // Note* The motor shield is not sign sensitive to my knowledge but I take the absolute value of the stepsneeded when it's negative just to be safe
  } else {                                              // Otherwise, if it's negative (backwards)
    myMotor->step(abs(stepsNeeded), BACKWARD, DOUBLE);  // Take that many stpes backwards
  }
  currentRow = wantedRow;  // After changing rows, the current row is now the row we wanted
}

int stateCheck(int row, int column) {  // This function checks the state of matter of a given const Element given by its row and column, and lights up it's LED based on it's state
  int state;
  float liquidTemp = periodicTable[row][column].meltingPoint;     // The liquid temp is its melting point
  float gasTemp = periodicTable[row][column].boilingPoint;        // The gas temp is its boiling point
  float tempKelvin = map(analogRead(tempPot), 0, 1023, 0, 6000);  // The temperature in kelvin is the potentiometer mapped to a value between 0 & 6000, which is the minimum and maximum temperature in Kelvin
  lcd.setCursor(0, 0);                                            // Set the LCD to column zero, row zero (it's zero indexed)
  lcd.print("Temp In Kelvin:");                                   // Print the string
  lcd.setCursor(0, 1);                                            // Set the cursor to the second row
  lcd.print(tempKelvin);                                          // Print the converted temperature
  // 0 = Solid, 1 = Liquid, 2 = Gas
  if (tempKelvin < gasTemp) {              // If the temperature is less than the boiling point
    if (tempKelvin >= liquidTemp) {        // But it's more than the melting point
      state = 1;                           // Then the state is liquid
    } else if (tempKelvin < liquidTemp) {  // Or if it's less than both
      state = 0;                           // Then it's a solid
    }
  } else if (tempKelvin >= gasTemp) {  // But if it is higher than the boiling point
    state = 2;                         // Then it's a gas
  }
  return state;  // Return the state of matter
}

void updateRow() {                        // This function updates the rows, it handles reading the buttons and doing what they're supposed to do
  Serial.println("home button is");
  Serial.print(digitalRead(homeButton));
  if (digitalRead(homeButton) == HIGH) {  // If the homing button is pressed
    stepsMade = 0.0;                     // Reset the step tracker to 0
    Serial.println("homed");
  }
  for (int i = 2; i < 4; i++) {
    Serial.println("Row");
    Serial.print(i);
    Serial.print(digitalRead(rowPins[i]));
    delay(1000);
    if (digitalRead(rowPins[i]) == HIGH) {
      changeRow(i);
      Serial.println("Changing to row");
      Serial.print(i);
    }
  }
  // If any trend buttons are pressed, then set the trend wanted to the respective trend and light it up given that for each column
  if (digitalRead(mmButton) == HIGH) {
    trendWanted = molarMass;
    for (int i = 0; i < 18; i++) {
      leds[i] = lightUp(currentRow, i, trendWanted);
    }
  } else if (digitalRead(mpButton) == HIGH) {
    trendWanted = meltingPoint;
    for (int i = 0; i < 18; i++) {
      leds[i] = lightUp(currentRow, i, trendWanted);
    }
  } else if (digitalRead(bpButton) == HIGH) {
    trendWanted = boilingPoint;
    for (int i = 0; i < 18; i++) {
      leds[i] = lightUp(currentRow, i, trendWanted);
    }
  } else if (digitalRead(enButton) == HIGH) {
    trendWanted = electroNegativity;
    for (int i = 0; i < 18; i++) {
      leds[i] = lightUp(currentRow, i, trendWanted);
    }
  } else if (digitalRead(ieButton) == HIGH) {
    trendWanted = ionizationEnergy;
    for (int i = 0; i < 18; i++) {
      leds[i] = lightUp(currentRow, i, trendWanted);
    }
  } else if (digitalRead(arButton) == HIGH) {
    trendWanted = atomicRadius;
    for (int i = 0; i < 18; i++) {
      leds[i] = lightUp(currentRow, i, trendWanted);
    }
  } else if (digitalRead(stateButton) == HIGH) {
    for (int i = 0; i < 18; i++) {            // For each column
      int state = stateCheck(currentRow, i);  // Check the state
      switch (state) {                        // If it's a solid make it grey
        case 0:
          leds[i] = CRGB::Gainsboro;
          break;
        case 1:  // If it's a liquid make it blue
          leds[i] = CRGB::RoyalBlue;
          break;
        case 2:  // If it's a gas make it red
          leds[i] = CRGB::Tomato;
          break;
      }
    }
  }
}

void setup() {
  Serial.begin(9600);                                                                              // Begin the serial monitor
  lcd.init();                                                                                      // Begin the LCD
  lcd.backlight();                                                                                 // Turn on the backlight
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);  //Initialize LED strips
  FastLED.setBrightness(BRIGHTNESS);                                                               // Set the brightness
  AFMS.begin();                                                                                    // Begin the motor
  myMotor->setSpeed(20);                                                                           // Set the motor speed
  for (int i = 0; i < numRows; i++) {
    pinMode(rowPins[i], INPUT);
  }
  pinMode(homeButton, INPUT);
  pinMode(mmButton, INPUT);
  pinMode(mpButton, INPUT);
  pinMode(bpButton, INPUT);
  pinMode(enButton, INPUT);
  pinMode(ieButton, INPUT);
  pinMode(arButton, INPUT);
  pinMode(stateButton, INPUT);
  pinMode(tempPot, INPUT);
}

void loop() {
  updateRow();
}
