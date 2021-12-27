#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
#include "main.h"
}

#define BACKGROUND_WIDTH 5760
#define BACKGROUND_HEIGHT 800
#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define UNICORN_SIZE 95
#define UNICORN_VELOCITY 4
#define MAX_DOLPHINS_NUMBER 10
#define COLLIDERS_NUMBER 6
#define FLOOR_NUMBER 9
#define FPS 60
#define FRAME_DELAY 1000/FPS
#define HEART_SIZE 30
#define ANIMATION_SPEED 10
#define MILESTONE 3000

struct xy
{
	int x;
	int y;
};
// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x; //- sprite->w / 2;
	dest.y = y; //- sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostok¹ta o d³ugoœci boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

void Death(int &x, int &y,int &velx,int &vely,double &t,int &life, int &menu,int &death, int &points,int &scroll,int&d) {
	x = UNICORN_SIZE;
	y = SCREEN_HEIGHT-UNICORN_SIZE;
	velx = 0;
	vely = 0;
	t = 0;
	life--;
	death = 0;
	points = 0;
	scroll = 0;
	if (life == 0) {
		d = 0;
		life = 3;
		menu = 1;
	}
}
void newGame(int& x, int& y, int& velx, int& vely, double& t,int &life, int& menu,int &points, int &scroll,int &d) {
	x = UNICORN_SIZE;
	y = SCREEN_HEIGHT - UNICORN_SIZE;
	velx = 0;
	vely = 0;
	t = 0;
	life = 3;
	points = 0;
	scroll = 0;
	d = 0;
}
void Freeing(SDL_Surface* charset, SDL_Surface* screen, SDL_Texture* scrtex, SDL_Renderer* renderer, SDL_Window* window)
{
	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}
// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	int t1, t2, quit, frames, rc, jump = 0, d = 0, action = 0, life = 3, frameTime, menu = 1, death = 0, y = 0,scroll=0,points=0;
	double delta, worldTime, fpsTimer, fps, distance, duration = 0,dolphinsSpeed=0;
	SDL_Event event;
	SDL_Surface* screen, * charset, * background, * unicorn[7], * heart1, * heart2, *dolphin;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	xy pos, vel;
	SDL_Rect unicorn_hitbox;
	SDL_Rect collider[2][COLLIDERS_NUMBER + FLOOR_NUMBER];
	SDL_Rect floor[2][FLOOR_NUMBER];
	SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	pos.x = UNICORN_SIZE, pos.y = SCREEN_HEIGHT - UNICORN_SIZE;
	vel.x = 0, vel.y = 0;

	// okno konsoli nie jest widoczne, je¿eli chcemy zobaczyæ
	// komunikaty wypisywane printf-em trzeba w opcjach:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// zmieniæ na "Console"
	// console window is not visible, to see the printf output
	// the option:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// must be changed to "Console"
	printf("wyjscie printfa trafia do tego okienka\n");
	printf("printf output goes here\n");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	// tryb pe³noekranowy / fullscreen mode
//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
//	                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		&window, &renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Szablon do zdania drugiego 2017");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);



	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	SDL_SetColorKey(charset, true, 0x000000);

	background = SDL_LoadBMP("./graphics/background.bmp");
	unicorn[0] = SDL_LoadBMP("./graphics/u1.bmp");
	unicorn[1] = SDL_LoadBMP("./graphics/u2.bmp");
	unicorn[2] = SDL_LoadBMP("./graphics/u3.bmp");
	unicorn[3] = SDL_LoadBMP("./graphics/u4.bmp");
	unicorn[4] = SDL_LoadBMP("./graphics/u5.bmp");
	unicorn[5] = SDL_LoadBMP("./graphics/u6.bmp");
	unicorn[6] = SDL_LoadBMP("./graphics/u7.bmp");
	heart1 = SDL_LoadBMP("./graphics/heart1.bmp");
	heart2 = SDL_LoadBMP("./graphics/heart2.bmp");
	dolphin = SDL_LoadBMP("./graphics/dolphin.bmp");
	for (int i = 0;i < 7;i++) {
		if ((unicorn[i] == NULL)||(background==NULL)||(heart1==NULL)||(heart2==NULL)||(charset==NULL)||(dolphin==NULL)){
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
}

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);


	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	distance = 0;

	while (!quit) {
		//menu glowne
		while (menu == 1) {
			SDL_FillRect(screen, NULL, niebieski);
			sprintf(text, "MENU GLOWNE");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
			sprintf(text, "Esc - wyjscie");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 46, text, charset);
			sprintf(text, "\156 - nowa gra");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 66, text, charset);
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_KEYDOWN)
				{
					if (event.key.keysym.sym == SDLK_ESCAPE)return 0;
					else if (event.key.keysym.sym ==SDLK_n) {
						t1 = SDL_GetTicks();
						life = 3;
						menu = 0;
						break;
					}
				}
			}
			SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
		//zaleznosc pozycji od predkosci
		pos.x += vel.x;
		if (pos.x < 0)
		{
			pos.x -= vel.x;
		}
		pos.y += vel.y;
		if (pos.y < 0)
		{
			pos.y -= vel.y;
		}

		//pozycja kamery
		camera.x = pos.x;
		camera.y = pos.y - SCREEN_HEIGHT / 2 + UNICORN_SIZE/2;
		if (camera.x < 0)camera.x = 0;
		if (camera.y < 0)camera.y = 0;
		if (camera.x > BACKGROUND_WIDTH - camera.w) { 
			camera.x = 0;
			pos.x = 0;
			scroll++; 
		}
		if (camera.y > BACKGROUND_HEIGHT - camera.h)camera.y = BACKGROUND_HEIGHT - camera.h;
		printf("%d\n", pos.x);

		t2 = SDL_GetTicks();

		// w tym momencie t2-t1 to czas w milisekundach,
		// jaki uplyna³ od ostatniego narysowania ekranu
		// delta to ten sam czas w sekundach
		// here t2-t1 is the time in milliseconds since
		// the last screen was drawn
		// delta is the same time in seconds
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		worldTime += delta;


		SDL_FillRect(screen, NULL, czarny);

		DrawSurface(screen, background, -camera.x, -camera.y);

		unicorn_hitbox = { pos.x,pos.y,UNICORN_SIZE,UNICORN_SIZE };
		floor[0][0] = {100,500,600,50};
		floor[1][0] = { BACKGROUND_WIDTH - SCREEN_WIDTH+100,500,600,50 };
		floor[0][1] = {400,450,300,50};
		floor[1][1] = { BACKGROUND_WIDTH - SCREEN_WIDTH+ 400,450,300,50 };
		floor[0][2] = { 900,600,350,50 };
		floor[1][2] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 900,600,350,50 };
		floor[0][3] = { 1600,500,500,50 };
		floor[1][3] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 1600,500,500,50 };
		floor[0][4] = { 2500,300,500,50 };
		floor[1][4] = { BACKGROUND_WIDTH - SCREEN_WIDTH+2500,300,500,50 };
		floor[0][5] = { 2600,650,500,50 };
		floor[1][5] = { BACKGROUND_WIDTH - SCREEN_WIDTH+2600,650,500,50 };
		floor[0][6] = { 2800,250,200,50 };
		floor[1][6] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 2800,250,200,50 };
		floor[0][7] = { 3500,400,600,50 };
		floor[1][7] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 3500,400,600,50 };
		floor[0][8] = { 4500,700,300,50 };
		floor[1][8] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 4500,700,300,50 };
		
		collider[0][0] = { 100,515,600,35 };
		collider[1][0] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 100,515,600,35 };
		collider[0][1] = { 400,465,300,35 };
		collider[1][1] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 400,465,300,35 };
		collider[0][2] = { 900,615,350,35 };
		collider[1][2] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 900,615,350,35 };
		collider[0][3] = { 1600,515,500,35 };
		collider[1][3] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 1600,515,500,35 };
		collider[0][4] = { 2500,315,500,35 };
		collider[1][4] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 2500,315,500,35 };
		collider[0][5] = { 2600,665,500,35 };
		collider[1][5] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 2600,665,500,35 };
		collider[0][6] = { 2800,265,200,35 };
		collider[1][6] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 2800,265,200,35 };
		collider[0][7] = { 3500,415,600,35 };
		collider[1][7] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 3500,415,600,35 };
		collider[0][8] = { 4500,715,300,35 };
		collider[1][8] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 4500,715,300,35 };
		collider[0][9] = { 1000,0,100,400 };
		collider[1][9] = { BACKGROUND_WIDTH-SCREEN_WIDTH+1000,0,100,400 };
		collider[0][10] = { 2000,0,100,250 };
		collider[1][10] = { BACKGROUND_WIDTH - SCREEN_WIDTH+2000,0,100,250 };
		collider[0][11] = { 100,0,100,200 };
		collider[1][11] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 100,0,100,200 };
		collider[0][12] = { 2900,350,80,160 };
		collider[1][12] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 2900,350,80,160 };
		collider[0][13] = { 3800,350,50,50 };
		collider[1][13] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 3800,350,50,50 };
		collider[0][14] = { 2600,600,50,50 };
		collider[1][14] = { BACKGROUND_WIDTH - SCREEN_WIDTH + 2600,600,50,50 };
		
		for (int i = 0;i < 2;i++) {
			for (int j = 0;j < (COLLIDERS_NUMBER + FLOOR_NUMBER);j++)SDL_FillRect(background, &collider[i][j], czerwony);
		}
		for (int i = 0;i < 2;i++) {
			for (int j = 0;j < FLOOR_NUMBER ;j++)SDL_FillRect(background, &floor[i][j], niebieski);
		}
		
		fpsTimer += delta;
		if (fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		};

		//wykrywanie smierci jednorozca
		for (int i = 0;i < FLOOR_NUMBER+COLLIDERS_NUMBER;i ++) {
			if (SDL_HasIntersection(&unicorn_hitbox, &collider[0][i])||pos.y>BACKGROUND_HEIGHT) {
				if (life > 1) {
					SDL_FillRect(screen, NULL, czerwony);
					sprintf(text, "STRACILES ZYCIE");
					DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
					sprintf(text, "Zdobyte punkty: %d", points);
					DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 46, text, charset);
					sprintf(text, "Czy chcesz kontynuowac?");
					DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 66, text, charset);
					sprintf(text, "Enter-kontynuuj gre");
					DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 86, text, charset);
					sprintf(text, "Esc-wyjscie z gry");
					DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 106, text, charset);
					while (death == 0) {
						while (SDL_PollEvent(&event)) {
							if (event.type == SDL_KEYDOWN)
							{
								if (event.key.keysym.sym == SDLK_ESCAPE)return 0;
								else if (event.key.keysym.sym == SDLK_RETURN) {
									t1 = SDL_GetTicks();
									death = 1;
									break;
								}
							}
						}
						SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
						SDL_RenderCopy(renderer, scrtex, NULL, NULL);
						SDL_RenderPresent(renderer);
					}
				}
				Death(pos.x, pos.y, vel.x,vel.y,worldTime,life,menu,death,points,scroll,d);
			}
		}
		//ustawianie pozycji jednorozca na platformie
		for (int i = 0;i < FLOOR_NUMBER;i++) {
			if ((SDL_HasIntersection(&unicorn_hitbox, &floor[0][i]))&& (!SDL_HasIntersection(&unicorn_hitbox, &collider[0][i]))) {
				pos.y = (floor[0][i].y - UNICORN_SIZE);
				jump = 0;
			}
		}
		//animacje w zaleznosci od akcji 
		if(d==0)DrawSurface(screen, unicorn[0], pos.x - camera.x, pos.y - camera.y);
		else if((action==1)||(pos.y<y)){ 
			DrawSurface(screen, unicorn[4], pos.x - camera.x, pos.y - camera.y); 
		}
		else if(pos.y>y){ 
			DrawSurface(screen, unicorn[5], pos.x - camera.x, pos.y - camera.y); 
		}
		else if(action==2){ 
			DrawSurface(screen, unicorn[6], pos.x - camera.x, pos.y - camera.y); 
		}
		else if(vel.x>0){
			DrawSurface(screen, unicorn[int((ceil(ANIMATION_SPEED*worldTime)))%4], pos.x - camera.x, pos.y - camera.y);
		}
		else DrawSurface(screen, unicorn[0], pos.x - camera.x, pos.y - camera.y);
		//rysowanie serc i paska z informacjami u góry
		for(int i=0;i<3;i++)DrawSurface(screen, heart2, i*HEART_SIZE,SCREEN_HEIGHT-30);
		for (int i = 0;i < life;i++)DrawSurface(screen, heart1, i * HEART_SIZE, SCREEN_HEIGHT - 30);
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		sprintf(text, "Czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		if(d==0)sprintf(text, " \030 - gora, \031 - dol,\032 - lewo, \033 - prawo");
		if (d == 1)sprintf(text, "z - skok, x - zryw");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 20, text, charset);
		sprintf(text, "d - zmiana sterowania, \156 - nowa gra, Esc - wyjscie");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 30, text, charset);
		points = int(0.5*(scroll * (BACKGROUND_WIDTH - camera.w) + pos.x-UNICORN_SIZE));
		sprintf(text,"%d",points);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 50, text, charset);
		for (int i = 1;i < MAX_DOLPHINS_NUMBER;i++) {
			dolphinsSpeed += delta;
			if (points > i * MILESTONE)DrawSurface(screen, dolphin, i*50, SCREEN_HEIGHT-100*sin(dolphinsSpeed/2));
		}
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// obs³uga zdarzeñ (o ile jakieœ zasz³y) / handling of events (if there were any)
		while (SDL_PollEvent(&event)) {
			if (d == 0) {
				switch (event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if ((event.key.keysym.sym == SDLK_UP) && (event.key.repeat == 0)) vel.y -= UNICORN_VELOCITY;
					else if ((event.key.keysym.sym == SDLK_DOWN) && (event.key.repeat == 0)) vel.y += UNICORN_VELOCITY;
					else if ((event.key.keysym.sym == SDLK_LEFT) && (event.key.repeat == 0)) vel.x -= UNICORN_VELOCITY;
					else if ((event.key.keysym.sym == SDLK_RIGHT) && (event.key.repeat == 0)) vel.x += UNICORN_VELOCITY;
					else if (event.key.keysym.sym == SDLK_n)newGame(pos.x, pos.y,vel.x,vel.y, worldTime,life,menu,points,scroll,d);
					else if (event.key.keysym.sym == SDLK_d)d = 1;
					break;
				case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_UP)vel.y = 0;
				else if (event.key.keysym.sym == SDLK_DOWN)vel.y = 0;
				else if (event.key.keysym.sym == SDLK_LEFT)vel.x = 0;
				else if (event.key.keysym.sym == SDLK_RIGHT)vel.x = 0;
				break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
				break;
			}
			else if (d == 1) {
				switch (event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if ((event.key.keysym.sym == SDLK_z) && (event.key.repeat == 0)&&(action!=2)&&(duration<0.5)&&(jump<2)) {
						vel.y = - 2*UNICORN_VELOCITY;
						jump++;
						action = 1;
					}
					else if ((event.key.keysym.sym == SDLK_x) && (event.key.repeat == 0)&&(action!=1)&&(duration<0.5)) { 
						vel.x = 3 * (UNICORN_VELOCITY + int(0.15 * worldTime));
						jump = 1;
						vel.y = 0;
						action = 2; 
					}
					else if (event.key.keysym.sym == SDLK_n) { vel.x = 0;
					vel.y = 0;
					newGame(pos.x, pos.y,vel.x,vel.y, worldTime,life,menu,points,scroll,d); 
					}
					else if (event.key.keysym.sym == SDLK_d) { 
						vel.y = 0;
						vel.x = 0;
						d = 0;
					}
					break;
				case SDL_KEYUP:
					if (event.key.keysym.sym == SDLK_z) {
						action = 0;
						duration = 0;
					}
					else if (event.key.keysym.sym == SDLK_x) {
						action = 0;
						duration = 0;
					}
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
				break;
			};
		};frames++;
		//defaultowa akcja i czas trwania wcisniecia
		if ((action == 0)&&(d==1)) {
			vel.x = UNICORN_VELOCITY + int(0.15*worldTime);
			vel.y = UNICORN_VELOCITY;
		}
		if ((d==1)&&((action==1)||(action==2))) {
			duration += delta;
			if (duration > 0.5) {
				duration = 0;
				action = 0;
			}
		}
		frameTime = SDL_GetTicks() - t2;
		if (FRAME_DELAY > frameTime)
		{
			SDL_Delay(FRAME_DELAY - frameTime);
		}
		y = pos.y;
	};

	Freeing(charset, screen, scrtex, renderer, window);

	SDL_Quit();
	return 0;
	};