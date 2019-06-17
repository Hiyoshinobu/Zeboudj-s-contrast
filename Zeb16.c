
#include <stdio.h>
#include <stdlib.h>

void resolution(FILE *p, int *X, int *Y)
{
	fseek(p, 0, SEEK_SET);
	fscanf(p, "%d", X);
	fscanf(p, "%d", Y);
	printf("X, Y = %d, %d\n", *X, *Y);
	fseek(p, 0, SEEK_SET);
}

void lim_int(unsigned char **tab_interieur, unsigned char **contraste, int X, int Y, int *E, int *I)
{
	int i, j;
	for(i=0;i<X;i++)
		for(j=0;j<Y;j++)
		{
			if(tab_interieur[i][j]==1)
				*I += (int) contraste[i][j];
			else if(tab_interieur[i][j]==0)
				*E += (int) contraste[i][j];
		}
}

float CRI(int E, int I)
{
	if(I<E && I>0)
		return (float) 1-(((float)I)/((float)E));
	else if(I==0)
		return (float) E;
	else
		return (float) 0;
}

int voisin(unsigned char** tab, int x, int y, int X, int Y)
{
	int tmp[8]={0}, i=0, schlag=0;
	if(y>0)
	{
		if(x>0)
			tmp[0]=tab[x][y]-tab[x-1][y-1];
		tmp[1]=tab[x][y]-tab[x][y-1];
		if(x<X-1)
			tmp[2]=tab[x][y]-tab[x+1][y-1];
	}
		if(x>0)
			tmp[3]=tab[x][y]-tab[x-1][y];
		if(x<X-1)
			tmp[4]=tab[x][y]-tab[x+1][y];
		if(y<Y-1){
			if(x>0)
				tmp[5]=tab[x][y]-tab[x-1][y+1];
			tmp[6]=tab[x][y]-tab[x][y+1];
			if(x<X-1)
				tmp[7]=tab[x][y]-tab[x+1][y+1];
		}
		for(i=0;i<8;i++)
			if(abs(tmp[i])>schlag)
				schlag=tmp[i];
	return schlag;
}

int interieur(unsigned char** tab, int x, int y, int X, int Y, int seuil)
//Retourn 1 si le pixel est intérieur, par rapport à ses voisins
{
	int tmp[9]={0}, i, schlag=0;
	if(y>0){
		if(x>0)
			tmp[0]=tab[x-1][y-1];
		tmp[1]=tab[x][y-1];
		if(x<X-1)
			tmp[2]=tab[x+1][y-1];
	}
	if(x>0)
		tmp[3]=tab[x-1][y];
	tmp[4]=tab[x][y];
	if(x<X-1)
		tmp[5]=tab[x+1][y];
	if(y<Y-1)
	{
		if(x>0)
			tmp[6]=tab[x-1][y+1];
		tmp[7]=tab[x][y+1];
		if(x<X-1)
			tmp[8]=tab[x+1][y+1];}
	for(i=0;i<9;i++)
	{
		if(tmp[i]>seuil)
			schlag+=0;
		if(tmp[i]<seuil)
			schlag+=1;
	}
	return (schlag%9 == 0);//schlag%9==0 <==> pixel est interieur
}


void sauveTab(FILE *p, int X, int Y, unsigned char **tab)
{
  int i,j;
  int debug=0;

  fseek(p, 256, SEEK_SET);
  debug += 256;
  fseek(p, X*(Y/4), SEEK_CUR);
  debug += X*(Y/4);
  for(i=0;i<X/2;i++)
  {
    fseek(p, X/4, SEEK_CUR);
    debug+=X/4;
    for(j=0;j<Y/2;j++)
    {
      tab[i][j]=(unsigned char) fgetc(p);
      debug++;
    }
    fseek(p, X/4, SEEK_CUR);
    debug+=X/4;
  }
  printf("%d\n%d\n", debug, 256 + X*Y/4);
  fseek(p, 0, SEEK_SET);
}

void pixToTab16(unsigned char*** tab, int X, int Y, FILE *input)
{
	int i=0, DEBUG=256,X4 = X/4,Y4 = Y/4;
	fseek(input, 256, SEEK_SET);
	/*
	for(i=0;i<16;i++)
		for(j=0;j<X/4;j++)
			for(k=0;k<Y/4;k++)
				tab[i][j][k] = (unsigned char) fgetc(input);//tab[numtab][][]

	*/
	printf("DEBUG\n");
	//for(i=0;i<16;i++){
	for(i;i<X*Y;i++)
		tab[(i/Y4)+4*(i/Y4)][i/Y4][i%Y4]=(unsigned char) fgetc(input);
}

int parseCarre(unsigned char **carre, int X, int Y)
{
	int i,j,seuil, seuilmax=0, E=0, I=0, surface=0;
	unsigned char **tab_interieur;
	tab_interieur = (unsigned char**) malloc(sizeof(unsigned char*)*(X));
	for(i=0;i<X/2;i++)
		tab_interieur[i] = (unsigned char*) malloc(sizeof(unsigned char)*(Y));

	for(seuil=50;seuil<=180;seuil++)
	{
		E=0;I=0;surface=0;
		for(i=0;i<X;i++){
			for(j=0;j<Y;j++){
				tab_interieur[i][j]=(unsigned char) interieur(carre, i, j, X, Y, seuil);
				surface += tab_interieur[i][j];
				if(tab_interieur)I++;else E++;
			}
		}
		if (CRI(E, I) > seuilmax)
			seuilmax=CRI(E,I);
	}

	return seuilmax;
}

void binCarre(unsigned char **carre, int X, int Y, int seuil)
//Binarise un carre de X par Y avec seuil comme limite
{
	int i,j;
	for(i=0;i<X;i++)
		for(j=0;j<Y;j++)
			carre[i][j]=256*(seuil>carre[i][j]);
}

int main()
{
	FILE *input, *output;
	int dimX, dimY, i, j, seuilmax;
	unsigned char ***tab;
	input=fopen("muscle.lena","r");
	output=fopen("output.lena","w");
	resolution(input, &dimX, &dimY);

	//-----------------------DEF TAB--------------------//

	tab = (unsigned char***) malloc(sizeof(unsigned char**)*(16));
	for(i=0;i<16;i++)
	{
		tab[i]=(unsigned char**) malloc(sizeof(unsigned char*)*(dimX/4));
		for(j=0;j<dimX/4;j++)
		{
			tab[i][j] = (unsigned char*) malloc(sizeof(unsigned char)*(dimY/4));
		}
	}
	printf("%d %d\n", dimX, dimY);
	//--------------------Main program-----------------//
	pixToTab16(tab, dimX, dimY, input);
	for(i=0;i<16;i++)
	{
		seuilmax = parseCarre(tab[i], dimX, dimY);
	}
}
