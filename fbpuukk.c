#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <math.h>
#include <time.h>
#include <signal.h>

int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
char *fbp = 0;
#define xres vinfo.xres
#define yres vinfo.yres

char getch()
{
	char buf=0;
	struct termios old={0};
	fflush(stdout);
	if(tcgetattr(0, &old)<0)
	{
		perror("tcsetattr()");
	}
	old.c_lflag&=~ICANON;
	old.c_lflag&=~ECHO;
	old.c_cc[VMIN]=1;
	old.c_cc[VTIME]=0;
	if(tcsetattr(0, TCSANOW, &old)<0)
	{
		perror("tcsetattr ICANON");
	}
	if(read(0,&buf,1)<0)
	{
		perror("read()");
	}
	old.c_lflag|=ICANON;
	old.c_lflag|=ECHO;
	if(tcsetattr(0, TCSADRAIN, &old)<0)
	{
		perror ("tcsetattr ~ICANON");
	}
	if(buf=='\n')
	{
		buf=' ';
	}
	return buf;
}

int pixel(int x, int y, int r, int g, int b)
{
	long int location = 0;
	if(x>=xres || y>=yres)
	{
		printf("\rCommand: Error: it will fucking segfaut you dumb fuck");
		//perror("Error: it will fucking segfault you dumb fuck");
		//exit(5);
		return(0);
	}
	location = (x+vinfo.xoffset)*(vinfo.bits_per_pixel/8)+(y+vinfo.yoffset)*finfo.line_length;
	if(vinfo.bits_per_pixel == 32)
	{
		*(fbp+location+0) = b;
		*(fbp+location+1) = g;
		*(fbp+location+2) = r;
		*(fbp+location+3) = 0;
	}
	else //lol assuming 16 bit color
	{
		//perror("Error: run it on a newer hardware, bitch.");
		//exit(6);
		int bk = b>>4;
		int gk = g>>4;
		int rk = r>>4;
		unsigned short int t = rk<<11 | gk<< 5 | bk;
		*((unsigned short int*)(fbp + location)) = t;
	}
	return(1);
}

int clear(int sx, int sy, int ex, int ey)
{
	for(int j=sy; j<ey; j++)
	{
		for(int i=sx; i<ex; i++)
		{
			pixel(i, j, 0, 0, 0);
		}
	}
}

int square(int sx, int sy, int ex, int ey, int r, int g, int b)
{
	if(sx >=xres ||ex>=xres || sy>=yres || ey>=yres)
	{
		printf("\rCommand: Error: that will fucking segfaut you dumb fuck");
		return(0);
	}
	for(int j=sy; j<ey; j++)
	{
		for(int i=sx; i<ex; i++)
		{
			pixel(i, j, r, g, b);
		}
	}
}

int animated(void)
{
	for(int ts=0; ts<800; ts++)
	{
		for(int j=96+ts; j<100+ts; j++)
		{
			for(int i=ts; i<100+ts; i++)
			{
				pixel(i, j, 255, 0, 0);
			}
		}
	usleep(1000);
	}
}

int smooth(void)
{
	for(int ts=0; ts<2073600; ts++)
	{
		char r=rand()>>23-1;
		pixel((int)((float)rand()/(float)RAND_MAX*(xres)-0.5), 96+(int)((float)rand()/(float)RAND_MAX*(yres-96.5)), r, r, r);
		usleep(10);
	}
}

int noise(void)
{
	for(int j=96; j<yres; j++)
	{
		for(int i=0; i<xres; i++)
		{
			char r=rand()>>23-1;
			pixel(i, j, r, r, r);
		}
	}
}

int mandelbrot(void)
{
	float x, y, xp, yp, t, r, g, b;
	float xs=3.5/(float)xres;
	float ys=2/(float)(yres-96);
	int iter=0;
	int max=256;
	for(int j=96; j<yres; j++)
	{
		for(int i=0; i<xres; i++)
		{
			iter=0;
			x=0;
			y=0;
			xp=i*xs-2.5;
			yp=(j-96)*ys-1.0;
			while(x*x + y*y < 4 && iter < max)
			{
				t=x*x-y*y+xp;
				y=2*x*y+yp;
				x=t;
				iter++;
			}
			int v = (float)iter/(float)max*256;
			pixel(i, j, v, v, v);
		}
	}
}

int iterated(void)
{
	for(int k=0; k<256; k++)
	{
		float x, y, xp, yp, t;
		float xs=3.5/(float)xres;
		float ys=2/(float)(yres-96);
		int iter=0;
		int max=k;
		for(int j=96; j<yres; j++)
		{
			for(int i=0; i<xres; i++)
			{
				iter=0;
				x=0;
				y=0;
				xp=i*xs-2.5;
				yp=(j-96)*ys-1.0;
				while(x*x + y*y < 4 && iter < max)
				{
					t=x*x-y*y+xp;
					y=2*x*y+yp;
					x=t;
					iter++;
				}
				int v = (float)iter/(float)max*256;
				pixel(i,j,v,v,v);
			}
		}
		k+=k;
	}
}

int zoom(void)
{
	for(int k=1; k<10000; k+=k)
	{
		float scale = 1-0.001*k;
		float x, y, xp, yp, t;
		float xs=3.5/xres*scale;
		float ys=2/(float)(yres-96)*scale;
		int iter=0;
		int max=32;
		for(int j=96; j<yres; j++)
		{
			for(int i=0; i<xres; i++)
			{
				iter=0;
				x=0;
				y=0;
				xp=(i*xs-2.5)*scale;
				yp=((j-96)*ys-1.0)*scale;
				while(x*x + y*y < 4 && iter < max)
				{
					t=x*x-y*y+xp;
					y=2*x*y+yp;
					x=t;
					iter++;
				}
				int v = (float)iter/(float)max*256;
				pixel(i,j,v,v,v);
			}
		}
	}
}

int main()
{
	srand(time(NULL));
	fbfd = open("/dev/fb0", O_RDWR);
	if(fbfd == -1)
	{
		perror("Error: cannot open frambuffer device");
		exit(1);
	}
	if(ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1)
	{
		perror("Error reading fixed information");
		exit(2);
	}
	if(ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1)
	{
		perror("Error reading variable information");
		exit(3);
	}
	fbp = (char *)mmap(0, xres * yres * vinfo.bits_per_pixel / 8, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if(fbp == NULL)
	{
		perror("Error: failed to map framebuffer device to memory");
		exit(4);
	}
	clear(0,0,xres,yres);
	int c;
	printf("Framebufferpuukotus v0.01\nEmil Fihlman\nGraffathon 2015\n");
	printf("0: Clear 1: Square 2: Animated 3: Smooth 4: Noise 5: Mandelbrot 6: Iterated 7: Zoom\n");
	printf("Command: ");
	while(1)
	{
		printf("\rCommand: %c",c=getch());
		printf("                                                            ");
		printf("\rCommand: ");
		switch(c)
		{
			case '0':
				clear(0, 96, xres, yres);
				break;
			case '1':
				square(500, 300, 700, 600, 0, 255, 0);
				break;
			case '2':
				animated();
				break;
			case '3':
				smooth();
				break;
			case '4':
				noise();
				break;
			case '5':
				mandelbrot();
				break;
			case '6':
				iterated();
				break;
			case '7':
				zoom();
				break;
			default:
				continue;
		}
	}
	clear(0, 0, xres, yres);
	munmap(fbp, xres * yres * vinfo.bits_per_pixel / 8);
	close(fbfd);
	return(0);
}
