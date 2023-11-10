#include <stdio.h>
#include <ctype.h>


/*************************************************
 *
 * dump some hex values -- also
 * show the ascii version of the dump.
 *
 *************************************************/ 
  
static char to_hex_char(int val)
{
   return("0123456789abcdef"[val & 0xf]);
}


void MsgEx_DumpHex(char *hextodump, int size)
{
  int i;
  char buf[80];
  int str_idx = 0;
  int chr_idx = 0;
  int count;
  int total;
  int tmp;

  if (hextodump == NULL)
    return;
  
  /* Initialize constant fields */
  memset(buf, ' ', sizeof(buf));
  buf[4]  = '|';
  buf[54] = '|';
  buf[72] = '\n';
  buf[73] = 0;
  
  count = 0;
  total = 0;
  for (i = 0; i < size; i++)
    {
      if (count == 0)
	{
          str_idx = 6;
          chr_idx = 56;
	  
          buf[0] = to_hex_char(total >> 8);
          buf[1] = to_hex_char(total >> 4);
          buf[2] = to_hex_char(total);
	}
      
      /* store the number */
      tmp = hextodump[i];
      buf[str_idx++] = to_hex_char(tmp >> 4);
      buf[str_idx++] = to_hex_char(tmp);
      str_idx++;
      
      /* store the character */
      buf[chr_idx++] = isprint(tmp) ? tmp : '.';
      
      total++;
      count++;
      if (count >= 16)
	{
          count = 0;
          printf("%s", buf);
	}
    }
  
  /* Print partial line if any */
  if (count != 0)
    {
      /* Clear out any junk */
      while (count < 16)
	{
          buf[str_idx]   = ' ';   /* MSB hex */
          buf[str_idx+1] = ' ';   /* LSB hex */
          str_idx += 3;
	  
          buf[chr_idx++] = ' ';
	  
          count++;
	}
      printf("%s", buf);
    }
}






