LCD 128 * 160 (1.8inch)
Connection via SPI
Power at 3.3V

GND: Ground \
VCC: 3.3-5V DC \
SCL: SPI Clock \
SDA: SPI Data \
RES: Reset \
DC: SPI Data / Select \
CS, BLK: Default floating (not connected), active LOW to shut down \

1.8TFT-LCD Arduino program usage instructions. This module is equipped with
The set of Arduino code is shown below
1. Put the UTFT.zip in the file into the libraries directory in the arduino installation directory.
Record and decompress
2 Open the .ino project file in any of the above three programs, click Compile Program,
If the library files are installed correctly, there will be no compilation errors. As shown in the picture\


For image (to get a penguin example):
void showimage(const unsigned char *p)
{
  	int i,j,k; 
	unsigned char picH,picL;
	dsp_single_colour(WHITE);
	
	for(k=0;k<3;k++)
	{
	   	for(j=0;j<3;j++)
		{	
			Lcd_SetRegion(40*j,40*k,40*j+39,40*k+39);
		    for(i=0;i<40*40;i++)
			 {	
			 	picL=*(p+i*2);
				picH=*(p+i*2+1);				
				LCD_WriteData_16Bit(picH<<8|picL);  						
			 }	
		 }
	}		
}
