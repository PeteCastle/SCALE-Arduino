#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

// Configure Serial1

#include <Base64.h>
#include <LCDWIKI_GUI.h> 
#include <LCDWIKI_KBV.h>
#include <SD.h>
#include <SPI.h>

#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
LCDWIKI_KBV mylcd(240,320,A3,A2,A1,A0,A4);
LiquidCrystal_I2C lcdi2c(0x27, 16, 2); // Change the 16, 2 to match the columns and rows of your LCD


#define PIXEL_NUMBER  (mylcd.Get_Display_Width()/4)
uint32_t bmp_offset = 0;
uint16_t s_width = mylcd.Get_Display_Width();  
uint16_t s_heigh = mylcd.Get_Display_Height();

void setup() {
  Serial.begin(115200); 
  Serial1.begin(115200);
   Serial.println("BEGIN");
   Serial.println(mylcd.Read_ID(), HEX);
   
   mylcd.Init_LCD();
   Serial.println("ENDBEGIN");

   mylcd.Set_Text_Size(2);
   mylcd.Fill_Screen(BLACK);  
   mylcd.Set_Rotation(1); 
  mylcd.Print_String("INTEGRATED MOSQUITO", 20, 10);
  mylcd.Print_String("DETECTION AND FUMIGATION", 20, 30);
   mylcd.Print_String("SYSTEM UTILIZING FASTER", 20, 50);
    mylcd.Print_String("CONVOLUTIONAL NEURAL ", 20, 70);
  mylcd.Print_String("NETWORKS", 20, 90);
  

  
  // Turn on the backlight (if available on your module)
  lcdi2c.init();
  lcdi2c.backlight();

  // Print a message to the LCD
  lcdi2c.setCursor(0, 0);
  lcdi2c.print("Detection Count:");
  lcdi2c.setCursor(0, 1);
  lcdi2c.print("21");

  File bmp_file;
  bmp_file = SD.open("logo.bmp");

  if(!analysis_bpm_header(bmp_file))
        {  
            mylcd.Set_Text_Back_colour(BLUE);
            mylcd.Set_Text_colour(WHITE);    
            mylcd.Set_Text_Size(1);
            mylcd.Print_String("bad bmp picture!",0,0);
            return;
  }
  draw_bmp_picture(bmp_file);
  
}


uint16_t read_16(File fp)
{
    uint8_t low;
    uint16_t high;
    low = fp.read();
    high = fp.read();
    return (high<<8)|low;
}

uint32_t read_32(File fp)
{
    uint16_t low;
    uint32_t high;
    low = read_16(fp);
    high = read_16(fp);
    return (high<<8)|low;   
 }
 

bool analysis_bpm_header(File fp)
{
    if(read_16(fp) != 0x4D42)
    {
      return false;  
    }
    //get bpm size
    read_32(fp);
    //get creator information
    read_32(fp);
    //get offset information
    bmp_offset = read_32(fp);
    //get DIB infomation
    read_32(fp);
    //get width and heigh information
    uint32_t bpm_width = read_32(fp);
    uint32_t bpm_heigh = read_32(fp);
    if((bpm_width != s_width) || (bpm_heigh != s_heigh))
    {
      return false; 
    }
    if(read_16(fp) != 1)
    {
        return false;
    }
    read_16(fp);
    if(read_32(fp) != 0)
    {
      return false; 
     }
     return true;
}

void draw_bmp_picture(File fp)
{
  uint16_t i,j,k,l,m=0;
  uint8_t bpm_data[PIXEL_NUMBER*3] = {0};
  uint16_t bpm_color[PIXEL_NUMBER];
  fp.seek(bmp_offset);
  for(i = 0;i < s_heigh;i++)
  {
    for(j = 0;j<s_width/PIXEL_NUMBER;j++)
    {
      m = 0;
      fp.read(bpm_data,PIXEL_NUMBER*3);
      for(k = 0;k<PIXEL_NUMBER;k++)
      {
        bpm_color[k]= mylcd.Color_To_565(bpm_data[m+2], bpm_data[m+1], bpm_data[m+0]); //change to 565
        m +=3;
      }
      for(l = 0;l<PIXEL_NUMBER;l++)
      {
        mylcd.Set_Draw_color(bpm_color[l]);
        mylcd.Draw_Pixel(j*PIXEL_NUMBER+l,i);
      }    
     }
   }    
}
void loop() {
  
}
