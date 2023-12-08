/* ht13.c
 * compile: gcc -O2 ht13.c -lftdi -lncurses -o ht13
 * this program transmits dmx-style rgb packets
 * modified to adjust to software PWM on the Mrfs
 * broadcasts from PC/Xbee to Mrf/Arduino
 *
 * really going to try constant broadcast this time
 * and... it flickers really badly on holds
 *
 * Benjamin Jeffery
 * University of Idaho
 * 11/11/2015
 * millisec() is copied from unicon/src/common/time.c
 * for testing purposes. Thanks Clint
 *
 * Some minor edits/cleanup by Lucas Jackson
 * University of Idaho
 * 10/27/2020
 */

#include <ftdi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h> 
//#include <sys/times.h>
#include <curses.h>

#define DOT   100000
#define DASH  300000
#define SLOW  500000
#define DORK  150000
#define DROOL 1000000
#define DAB   50000
#define SLP   40000

//static size_t encode(const uint8_t* source, size_t size, uint8_t* destination);
static size_t getEncodedBufferSize(size_t sourceSize);
long millisec();
uint8_t sorc[108] = { };
uint8_t dest[108] = { };

uint8_t convert_red(int);
uint8_t convert_green(int);
uint8_t convert_blue(int);

uint8_t create_patterns(char *glasses_information, uint8_t***, int**, int z); 
static int parse_ext(const struct dirent *dir);

void rotate13 (uint8_t arr[]);

int main() {
  struct ftdi_context *ftdi, *ftdi_2;
  struct ftdi_device_list *devlist, *curdev, *devlist_2, *curdev_2;
  //ftdi_free(ftdi);
  //ftdi_free(ftdi_2);
  char manufacturer[128], description[128];
  int retval = EXIT_SUCCESS;
  char letter;
  int i = 0;
  int j = 0;
  int k = 0;
  int ret = 0;
  int res;
  int nbytes;
  int f;
  int r = 224;
  int g = 28;
  int b = 3;
  int y = 252;
  size_t write_index;
  uint8_t temp[3] = { };
  uint8_t dPack[108] = { };
  uint8_t dbytePack[108] = {0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                            0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0};

  uint8_t whitePack[96] = {255,255,215, 255,255,215, 255,255,215, 255,255,215, 
                           255,255,215, 255,255,215, 255,255,215, 255,255,215, 
                           255,255,215, 255,255,215, 255,255,215, 255,255,215, 
                           255,255,215, 255,255,215, 255,255,215, 255,255,215, 
                           255,255,215, 255,255,215, 255,255,215, 255,255,215, 
                           255,255,215, 255,255,215, 255,255,215, 255,255,215, 
                           255,255,215, 255,255,215, 255,255,215, 255,255,215, 
                           255,255,215, 255,255,215, 255,255,215, 255,255,215};

  size_t n = sizeof(whitePack);
  size_t l = sizeof(dest);
  size_t m = getEncodedBufferSize(l);
  struct ftdi_context * ftdi_devices[2];
  struct ftdi_device_list * ftdi_device_list[2];
  // initialize two new ftdi devices
  if ((ftdi = ftdi_new()) == 0) {
      fprintf(stderr, "ftdi_new failed\n");
      return EXIT_FAILURE;
    } else {
    fprintf(stderr, "ftdi_new success\n");
  }
  // initialize second ftdi device
  if ((ftdi_2 = ftdi_new()) == 0) {
      fprintf(stderr, "ftdi_new failed\n");
      return EXIT_FAILURE;
    } else {
    fprintf(stderr, "ftdi_new success\n");
  }


  //Loop through the array and detect each ftdi device
  for(int i = 0; i < 2; i++)  {
    if (ftdi_usb_find_all(ftdi_devices[i], &ftdi_device_list[i], 0x0403, 0x6015) < 1) {
      if(ftdi_usb_find_all(ftdi_devices[i], &ftdi_device_list[i], 0x0403, 0x6001) < 1)  {
        printf("no ftdi devices found for ftdi device slot %d\n", i);
      }  
    }
    printf("\n%d\n", i);
  }

  // detect connected ftdi device(s)
  if (((ftdi_usb_find_all(ftdi, &devlist, 0x0403, 0x6015)) < 1) && ((ftdi_usb_find_all(ftdi_2, &devlist_2, 0x0403, 0x6001)) < 1)) {
    fprintf(stderr, "no ftdi devices found\n");
    fflush(stderr);
    ftdi_list_free(&devlist);
    ftdi_list_free(&devlist_2);
    ftdi_free(ftdi);
    ftdi_free(ftdi_2);
    return 1;
  } else {

  }

  //Track the currently assigned device number
  int device_number = 0;

  ftdi_usb_find_all(ftdi, &devlist, 0x0403, 0x6015);
  // loop through detected devices and attempt to get their information
  // This should loop though all FTDI devices that have the VID:PID of 0x403:0x6015
  i = 0;
  for (curdev = devlist; curdev != NULL; i++) {
      printf("Checking device: 1\n");
      if ((ret = ftdi_usb_get_strings(ftdi, curdev->dev, manufacturer, 128, description, 128, NULL, 0)) < 0) {
        fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        retval = EXIT_FAILURE;
        ftdi_list_free(&devlist);
        ftdi_free(ftdi);
      }
      else  {
        if(device_number == 0)  {
          ftdi_devices[device_number] = ftdi;
          ftdi_device_list[device_number] = curdev;
          device_number++;
        }
        else {
          ftdi_usb_find_all(ftdi_2, &devlist_2, 0x0403, 0x6001);
          ftdi_devices[device_number] = ftdi_2;
          ftdi_device_list[device_number] = devlist_2->next;
          device_number++;
        }
      }
    printf("Device One: Manufacturer: %s, Description: %s\n\n", manufacturer, description);
    curdev = curdev->next;
    }

    
  ftdi_usb_find_all(ftdi, &devlist, 0x0403, 0x6001);
  i = 0;
  for (curdev = devlist; curdev != NULL; i++) {
      printf("Checking device: 2\n");
      if ((ret = ftdi_usb_get_strings(ftdi, curdev->dev, manufacturer, 128, description, 128, NULL, 0)) < 0) {
        fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        retval = EXIT_FAILURE;
        ftdi_list_free(&devlist);
        ftdi_free(ftdi);
      }
      else  {
        if(device_number == 0)  {
          ftdi_devices[device_number] = ftdi;
          ftdi_device_list[device_number] = curdev;
          device_number++;
        }
        else {
          ftdi_usb_find_all(ftdi_2, &devlist_2, 0x0403, 0x6001);
          ftdi_devices[device_number] = ftdi_2;
          ftdi_device_list[device_number] = devlist_2->next;
          device_number++;
        }
      }
    printf("Device Two: Manufacturer: %s, Description: %s\n\n", manufacturer, description);
    curdev = curdev->next;
  }

  //convert back to seperate ftdi contexts (might be unnecessary)
  ftdi = ftdi_devices[0];
  devlist = ftdi_device_list[0];
  ftdi_2 = ftdi_devices[1];
  devlist_2 = ftdi_device_list[1];

  bool two_devices = true;
  if(device_number < 2) {
    two_devices = false;
  }

  // open ftdi context
  if ((ret = ftdi_usb_open_dev(ftdi, devlist->dev)) < 0)
    {
      fprintf(stderr, "unable to open ftdi: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
      ftdi_free(ftdi);
      return ret;            
    }
  else {
    fprintf(stderr, "ftdi_open successful\n");
  }

  // set the base bitrate/baudrate of the device(s)
  ret = ftdi_set_baudrate(ftdi, 57600);
  if (ret < 0) {
    fprintf(stderr, "unable to set baud rate: %d (%s).\n", ret, ftdi_get_error_string(ftdi));
  } else {
    printf("baudrate set.\n");
  }

if(two_devices == true) {
    if ((ret = ftdi_usb_open_dev(ftdi_2, devlist_2->dev)) < 0)
      {
        fprintf(stderr, "unable to open ftdi: %d (%s)\n", ret, ftdi_get_error_string(ftdi_2));
        ftdi_free(ftdi_2);
        return ret;            
      }
    else {
      fprintf(stderr, "ftdi_open two successful\n");
    }

    ret = ftdi_set_baudrate(ftdi_2, 57600);
    if (ret < 0) {
    fprintf(stderr, "unable to set baud rate: %d (%s).\n", ret, ftdi_get_error_string(ftdi_2));
    } else {
      printf("baudrate set.\n");
    }

    f = ftdi_set_line_property(ftdi_2, 8, STOP_BIT_1, NONE);
    if(f < 0) {
      fprintf(stderr, "unable to set line parameters: %d (%s).\n", ret, ftdi_get_error_string(ftdi));
    } else {
      printf("line parameters set.\n");
    }
  }


  // set parameters in the devices
  f = ftdi_set_line_property(ftdi, 8, STOP_BIT_1, NONE);
  if(f < 0) {
    fprintf(stderr, "unable to set line parameters: %d (%s).\n", ret, ftdi_get_error_string(ftdi));
  } else {
    printf("line parameters set.\n");
  }
  

  ftdi_list_free(&devlist);
  if(two_devices == true) {
    ftdi_list_free(&devlist_2);
  }
  printf("broadcasting.\n");

  FILE * test_file = fopen("test_ht13", "r");

  //open curses session for display purposes
  initscr();
  raw();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  noecho();
  attron(A_BOLD);
  

  //LEAVING OFF HERE it can now read an entire file but can't differentiate between patterns and time is not yet being saved.
  FILE *fp;
  long lSize;
  char *glasses_information;
//Read all files in folder searching for ht13 files
struct dirent **namelist;
struct dirent **filtered_list;
int z;

z = scandir(".", &namelist, parse_ext, alphasort);
if (z < 0) {
  perror("scandir");
  return 1;
}
else  {

}
//dimensions of the 3-d array
// For the information being collected there are z blocks of 2-d arrays, 30 rows max of patterns and 108 addresses per pattern.
//Allocate memory blocks based on number of files here **********************************
uint8_t ***test_converted_ht13 = (uint8_t***)malloc(z * sizeof(uint8_t**));
if (test_converted_ht13 == NULL)
{
  fprintf(stderr, "Not enough memory available");
  exit(0);
} 

for (int i = 0; i < z; i++)
{
  //30 is used here as a placeholder for maximum number of patterns allowed.
  test_converted_ht13[i] = (uint8_t**)malloc(30 * sizeof(uint8_t*));
  if (test_converted_ht13[i] == NULL)
  {
    fprintf(stderr, "Not enough memory available");
    exit(0);
  }
  //110 slots of memory are available to enable reading of the time.
  for (int q = 0; q < 110; q++)
  {
    test_converted_ht13[i][q] = (uint8_t*)malloc(108 * sizeof(uint8_t));
    if (test_converted_ht13[i][q] == NULL)  
    {
      fprintf(stderr, "Out of memory");
      exit(0);
    }
  }
}
//Allocate memory in the same way to hold the time values minus the step involving an extra 108 slots
int *time_converted_ht13[z];

if (time_converted_ht13 == NULL)
{
  fprintf(stderr, "Not enough memory available");
  exit(0);
} 

for (int i = 0; i < z; i++)
{
  //30 is used here as a placeholder for maximum number of patterns allowed.
  time_converted_ht13[i] = (int*)malloc(30 * sizeof(int));
  if (time_converted_ht13[i] == NULL)
  {
    fprintf(stderr, "Not enough memory available");
    exit(0);
  }
}

int pattern_total = 0;
char keybind[30];
for (int i = 0; i < 30; i++)  {
  keybind[i] = 'N';
}
//for all ht13 files make a pattern
printw("Hello, welcome to Ben's Halftime Toolkit!\n");
while (z--) {
    //printw("%s\n", namelist[z]->d_name);


  fp = fopen ( namelist[z]->d_name , "r" );
  if( !fp ) perror("ht13 file failed to open"),exit(1);

  fseek( fp , 0L , SEEK_END);
  lSize = ftell( fp );
  rewind( fp );

  /* allocate memory for entire content */
  glasses_information = calloc( 1, lSize+1 );
  if( !glasses_information ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

  /* copy the file into the buffer */
  if( 1!=fread( glasses_information , lSize, 1 , fp) )
    fclose(fp),free(glasses_information),fputs("entire read fails",stderr),exit(1);

  int keybind_location = strlen(glasses_information);
  keybind[z] = glasses_information[keybind_location-1];
  printw("%c for file: %s\n", keybind[z], namelist[z]->d_name);

  //This should be looped based on which file the program is on.
  create_patterns(glasses_information, test_converted_ht13, time_converted_ht13, z);
}
free(glasses_information);
  
  
  printw("\n");
  printw(" comma key <,> to stop loop\n");
  printw(" dot key <.> to quit");

  int q = 0;
  int key_pressed = 0;
  bool is_valid = false;
  // loop until a '.' character is detected
  do {
    q = 0;
    key_pressed = 0;
    is_valid = false;
    for(int i = 0; keybind[i]; i++)
    {
      if(letter == keybind[i])
      {
        is_valid = true;
        key_pressed = i;
        //printw("%d", key_pressed);
      }
    }
    letter = getch();
	  move(15,0);
	  printw("%c",(letter==-1 ? ' ' : letter));
	  move(15,0);
    //adjusting statements to try automatically generated patterns from reading from file.
    if(is_valid == true)
    {
      q = 0;
      //begin by writing the first pattern to the ftdi devices
      int loop = 0;
      clock_t start, current;

      while ((time_converted_ht13[key_pressed][q] != -1))
      {
        //printw("%d ", q);
      //The code just breaks without this usleep. I do not understand.
      usleep(20000);
      start = clock();
      nbytes = ftdi_write_data(ftdi, test_converted_ht13[key_pressed][q], m);
      //sleep for amount of time for specified pattern for the amount of time the pattern is to stay on
      //make the sleep interruptable no matter how long it is by checking for getch()
        long elapsed = 0;
        current = clock();
        //printw("%d %d", current, test_converted_ht13[letter-48][q]);
        while ((elapsed) < time_converted_ht13[key_pressed][q] && getch() != ',')
        {
          current = clock();
          elapsed = ((double)current - start) / CLOCKS_PER_SEC * 1000;
        }
      q++;
      nbytes = ftdi_write_data(ftdi, dbytePack, m);
      usleep(20000);
      }
    }
    else
    {
      usleep(DAB);
    }
    nbytes = ftdi_write_data(ftdi, dbytePack, m);
   // Draw a space over current character
    move(14,0);
    //printw(" ");
    refresh();
  } while (letter != '.');
  
  
  endwin();   // close curses session
  
  // close ftdi device connection
  if ((ret = ftdi_usb_close(ftdi)) < 0)
    {
      fprintf(stderr, "unable to close ftdi1: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
      return 1;
    }

  ftdi_free(ftdi);
  
  printf("End of program.\n");
  
  return 0;
} // END main

void rotate13 (uint8_t arr[]) {
    // This routine rotates the values in the lower 13 channels (channels 0-12)
    // since the flags are on channels 13 and 14, they are left alone
   uint8_t  r, g, b;
   int i, j, nrot;

   r = arr[0];
   g = arr[1];
   b = arr[2];

   nrot = 13;
   for(i = 3; i < nrot*3; i++) {
      j = i + 48;
      arr[i-3] = arr[i];
      arr[j-3] = arr[j];
     } // for i
 
   arr[nrot*3] = r;
   arr[nrot*3+1] = g;
   arr[nrot*3+2] = b;

  } // END rotate12


static size_t getEncodedBufferSize(size_t sourceSize) {
  size_t s;
  s = sourceSize + sourceSize / 254 + 1;
  printf("buffer size is : %zd.\n", s);
  return s;
}

bool find_ftdi()  {

}

uint8_t convert_blue(int color) 
{
  int picked = 0;
  uint8_t color_options[4] = {0, 85, 170, 255};
  for( int i = 0; i < 4; i++) {
    if(abs(color_options[i] - color) < 43)
      picked = i;
  }
  switch(picked)
  {
    case 0:
      return 0b00000000;
      break;
    case 1:
      return 0b00000001;
      break;
    case 2:
      return 0b00000010;
      break;
    case 3:
      return 0b00000011;
      break;
  }
}

uint8_t convert_green(int color)
{
  int picked = 0;
  uint8_t color_options[8] = {0, 37, 74, 111, 148, 185, 222, 255};
  for (int i = 0; i < 8; i++) {
    if(abs(color_options[i] - color) < 19)
      picked = i;
  }
  switch(picked)
  {
    case 0:
      return 0b00000000;
      break;
    case 1:
      return 0b00000100;
      break;
    case 2:
      return 0b00001000;
      break;
    case 3:
      return 0b00001100;
      break;
    case 4:
      return 0b00010000;
      break;
    case 5:
      return 0b00010100;
      break;
    case 6:
      return 0b00011000;
      break;
    case 7:
      return 0b00011100;
      break;
  }
}

uint8_t convert_red(int color)
{
  int picked = 0;
  uint8_t color_options[8] = {0, 37, 74, 111, 148, 185, 222, 255};
  for (int i = 0; i < 8; i++) {
    if(abs(color_options[i] - color) < 19)
      picked = i;
  }
  switch(picked)
  {
    case 0:
      return 0b00000000;
      break;
    case 1:
      return 0b00100000;
      break;
    case 2:
      return 0b01000000;
      break;
    case 3:
      return 0b01100000;
      break;
    case 4:
      return 0b10000000;
      break;
    case 5:
      return 0b10100000;
      break;
    case 6:
      return 0b11000000;
      break;
    case 7:
      return 0b11100000;
      break;
  }
}

//glasses_information will contain an entire files worth of text.
uint8_t create_patterns(char *glasses_information, uint8_t ***test_converted_ht13, int **time_converted_ht13, int z)  {
  for (int i = 0; i < 108; i++) {
    test_converted_ht13[z][0][i] = 0;
  }
  uint8_t converted_red = 0;
  uint8_t converted_green = 0;
  uint8_t converted_blue = 0;
  uint8_t rgb_color = 0;
  int current_pattern = 0;
  for (int i = 0; i < strlen(glasses_information); i++)
  {
    //find the address
    int address = 0;
    if((47 < glasses_information[i]) && (glasses_information[i] < 58))  {
      address = glasses_information[i] - 48;
      while((47 < glasses_information[i+1]) && (glasses_information[i+1] < 58))  {
        address = (address * 10) + (glasses_information[i+1] - 48);
        i++;
      }
      if(glasses_information[i+1] == 41)  {
        time_converted_ht13[z][current_pattern] = address;
        //printw("\n %d \n", address);
        //printw("\n %d \n", time_converted_ht13[0][current_pattern]);
        current_pattern++;
        time_converted_ht13[z][current_pattern + 1] = -1;
        for (int p = 0; p < 108; p++) {
          test_converted_ht13[z][current_pattern][p] = 0;
        }
      }
      else  {
        //figure out what color
        int color = 0;
        i = i + 2;
        //Red coloring is found by checking number until seperated by a "["
        converted_red = 0;
        converted_green = 0;
        converted_blue = 0;
        rgb_color = 0;
        while(glasses_information[i] != 91) {
          color = (color * 10) + (glasses_information[i] - 48);
          i++;
        }
        converted_red = convert_red(color);
        color = 0;
        i++;
        while(glasses_information[i] != 91) {
          color = (color * 10) + (glasses_information[i] - 48);
          i++;
        }
        converted_green = convert_green(color);
        color = 0;
        i++;
        while(glasses_information[i] != 124)  {
          color = (color * 10) + (glasses_information[i] - 48);
          i++;
        }
        converted_blue = convert_blue(color);
        color = 0;
        rgb_color = converted_red + converted_green + converted_blue;
        //use a function to condense this result into three bits to find red

        test_converted_ht13[z][current_pattern][address] = rgb_color;
        color = 0;
      }
    }
    address = 0;
  }
}

// when returning 1, scandir will put this dirent into the list
static int parse_ext(const struct dirent *dir)
{
  if(!dir)
    return 0;

  if(dir->d_type == DT_REG) {
    const char *ext = strrchr(dir->d_name,'.');
    if((!ext) || ext == dir->d_name)
      return 0;
    else  {
      if(strcmp(ext, ".ht13") == 0)
      return 1;
    }
  }
  return 0;
}