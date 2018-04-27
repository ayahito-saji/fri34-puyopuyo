#include <curses.h>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#define FIELD_WIDTH 6
#define FIELD_HEIGHT 12
#define int_len( ary ) sizeof(ary) / sizeof(int)
#define debug( object ) std::cout << object << std::endl; 

// ぷよの色データ
enum puyocolor {NONE, RED, BLUE, GREEN, YELLOW, PURPLE};
// ぷよの状態データ
enum puyostatus {UNCHECKED, CHECKED};

// ぷよクラス
class Puyo{
public:
    enum puyocolor color;   // ぷよの色
    enum puyostatus status; // ぷよの状態
    Puyo(){
        color = NONE;
    }
    Puyo(enum puyocolor color_){
        color = color_;
    }
    //ぷよの表示
    char view(){
        switch(color){
            case NONE:
	        attrset(COLOR_PAIR(0));
                return '*';
            case RED:
	        attrset(COLOR_PAIR(1));
                return 'R';
            case BLUE:
	        attrset(COLOR_PAIR(2));
                return 'B';
            case GREEN:
	        attrset(COLOR_PAIR(3));
                return 'G';
            case YELLOW:
	        attrset(COLOR_PAIR(4));
                return 'Y';
            case PURPLE:
	        attrset(COLOR_PAIR(5));
                return 'P';
            default:
	        attrset(COLOR_PAIR(0));
                return '?';
        }
    }
    static void initcolor(){
        init_pair(0, COLOR_WHITE, COLOR_BLACK);
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_BLUE, COLOR_BLACK);
        init_pair(3, COLOR_GREEN, COLOR_BLACK);
        init_pair(4, COLOR_YELLOW, COLOR_BLACK);
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    }
};
//落下中のぷよクラス
class HandlingPuyo: public Puyo{
public:
    int x, y;
    HandlingPuyo(){
        color = NONE;
        x = -1;
        y = -1;
    }
    HandlingPuyo(enum puyocolor color_, int x_, int y_){
        color = color_;
        x = x_;
        y = y_;
    }
};
// フィールドのデータ
class Field{
private:
    Puyo* field[FIELD_WIDTH * FIELD_HEIGHT];    //フィールドのぷよデータ
    HandlingPuyo h_puyo[2];                     //落下中のぷよデータ
public:
    Field(){
        //フィールドをNULLで初期化
        for(int i=0;i<FIELD_WIDTH * FIELD_HEIGHT;i++) field[i] = NULL;
    }
    //フィールドのぷよのアクセッサ―
    Puyo* getPuyo(int x, int y){
        return field[y * FIELD_WIDTH + x];
    }
    bool setPuyo(int x, int y, Puyo* puyo){
        if(field[y * FIELD_WIDTH + x] != NULL) return false;    //NULLでなければ失敗
        field[y * FIELD_WIDTH + x] = puyo;
        return true;
    }
    //フィールドの表示
    void view(int fx, int fy){
        //固定ぷよの表示
        Puyo* p;
        for(int y=0;y<FIELD_HEIGHT;y++){
            mvaddch(y+fy, fx, '#');
            for(int x=0;x<FIELD_WIDTH;x++){
                p = getPuyo(x,y);
                if(p!=NULL) mvaddch(y+fy, x+fx+1, (*p).view());
                else mvaddch(y+fy, x+fy+1, ' ');
	        attrset(COLOR_PAIR(0));
            }
            mvaddch(y+fy, FIELD_WIDTH+fx+1, '#');
        }
        for(int x=-1;x<FIELD_WIDTH+1;x++){
            mvaddch(FIELD_HEIGHT+fy, x+fx+1, '#');
        }
        //落下ぷよの表示
        if(h_puyo[0].y>=0) mvaddch(h_puyo[0].y+fy, h_puyo[0].x+fx+1, h_puyo[0].view());
        if(h_puyo[1].y>=0) mvaddch(h_puyo[1].y+fy, h_puyo[1].x+fx+1, h_puyo[1].view());
	
	attrset(COLOR_PAIR(0));
    }
    //フィールドに新しいぷよ生成
    void generate(){
        HandlingPuyo hp0((puyocolor) (rand()%5+1), 2, 0);
        HandlingPuyo hp1((puyocolor) (rand()%5+1), 3, 0);
        h_puyo[0] = hp0;
        h_puyo[1] = hp1;
    }
    //下移動
    bool controllDown(){
        if(h_puyo[0].y+1>=FIELD_HEIGHT || h_puyo[1].y+1>=FIELD_HEIGHT) return false;
	if(getPuyo(h_puyo[0].x, h_puyo[0].y+1)!=NULL || getPuyo(h_puyo[1].x, h_puyo[1].y+1)!=NULL) return false;
        h_puyo[0].y++;
        h_puyo[1].y++;
	return true;
    }
    //左移動
    bool controllLeft(){
        if(h_puyo[0].x-1<0 || h_puyo[1].x-1<0) return false;
	if(getPuyo(h_puyo[0].x-1, h_puyo[0].y)!=NULL || getPuyo(h_puyo[1].x-1, h_puyo[1].y)!=NULL) return false;
        h_puyo[0].x--;
        h_puyo[1].x--;
	return true;
    }
    //右移動
    bool controllRight(){
        if(h_puyo[0].x+1>=FIELD_WIDTH || h_puyo[1].x+1>=FIELD_WIDTH) return false;
	if(getPuyo(h_puyo[0].x+1, h_puyo[0].y)!=NULL || getPuyo(h_puyo[1].x+1, h_puyo[1].y)!=NULL) return false;
        h_puyo[0].x++;
        h_puyo[1].x++;
	return true;
    }
    //落下ぷよを個別に操作不能のまま落とす
    void uncontrollDown(){
        unsigned int delay = 0, ch;
        unsigned int waitCount = 5000;
	    int landed[2] = {false, false};
        while(!landed[0] || !landed[1]){
	        ch = getch();
	        if(delay == 0){
	            if(h_puyo[0].y+1<FIELD_HEIGHT){
                    if(getPuyo(h_puyo[0].x, h_puyo[0].y+1) == NULL){
                        if(h_puyo[0].x != h_puyo[1].x || h_puyo[0].y + 1 != h_puyo[1].y){
                            h_puyo[0].y++;
                        }else landed[0] = true;
                    }else landed[0] = true;
                }else landed[0] = true;
	            if(h_puyo[1].y+1<FIELD_HEIGHT){
                    if(getPuyo(h_puyo[1].x, h_puyo[1].y+1) == NULL){
                        if(h_puyo[0].x != h_puyo[1].x || h_puyo[1].y + 1 != h_puyo[0].y){
                            h_puyo[1].y++;
                        }else landed[1] = true;
                    }else landed[1] = true;
                }else landed[1] = true;
            }
            view(1,1);
	        delay = (delay + 1) % waitCount;
        }
    }
    //落下ぷよを左回転する
    bool controllLeftRotate(){
        if(h_puyo[1].y - h_puyo[0].y == -1){            //軸ぷよの上にある
            //左が障害物またはぷよが存在しなければ普通に回す
            if(h_puyo[0].x-1>=0){
                if(getPuyo(h_puyo[0].x-1, h_puyo[0].y)==NULL){
                    h_puyo[1].x = h_puyo[0].x - 1;
                    h_puyo[1].y = h_puyo[0].y;
                    return true;
                }
            }
            //右に障害物またはぷよが存在しなければ軸ぷよを一つ右にずらして回す
            if(h_puyo[0].x+1<FIELD_WIDTH){
                if(getPuyo(h_puyo[0].x+1, h_puyo[0].y)==NULL){
                    h_puyo[0].x = h_puyo[0].x + 1;
                    h_puyo[0].y = h_puyo[0].y;
                    h_puyo[1].x = h_puyo[0].x - 1;
                    h_puyo[1].y = h_puyo[0].y;
                    return true;
                }
            }
        }else if(h_puyo[1].x - h_puyo[0].x == -1){      //軸ぷよの左にある
            //下が障害物またはぷよが存在しなければ普通に回す
            if(h_puyo[0].y+1<FIELD_HEIGHT){
                if(getPuyo(h_puyo[0].x, h_puyo[0].y+1)==NULL){
                    h_puyo[1].x = h_puyo[0].x;
                    h_puyo[1].y = h_puyo[0].y + 1;
                    return true;
                }
            }
            //下に障害物があれば、上に一つずらして回す
            h_puyo[0].x = h_puyo[0].x;
            h_puyo[0].y = h_puyo[0].y - 1;
            h_puyo[1].x = h_puyo[0].x;
            h_puyo[1].y = h_puyo[0].y + 1;
            return true;
           
        }else if(h_puyo[1].y - h_puyo[0].y == 1){       //軸ぷよの下にある
            //右が障害物またはぷよが存在しなければ普通に回す
            if(h_puyo[0].x+1<FIELD_WIDTH){
                if(getPuyo(h_puyo[0].x+1, h_puyo[0].y)==NULL){
                    h_puyo[1].x = h_puyo[0].x + 1;
                    h_puyo[1].y = h_puyo[0].y;
                    return true;
                }
            }
            //左に障害物またはぷよが存在しなければ軸ぷよを一つ左にずらして回す
            if(h_puyo[0].x-1>=0){
                if(getPuyo(h_puyo[0].x-1, h_puyo[0].y)==NULL){
                    h_puyo[0].x = h_puyo[0].x - 1;
                    h_puyo[0].y = h_puyo[0].y;
                    h_puyo[1].x = h_puyo[0].x + 1;
                    h_puyo[1].y = h_puyo[0].y;
                    return true;
                }
            }
        }else if(h_puyo[1].x - h_puyo[0].x == 1){       //軸ぷよの右にある
            h_puyo[1].x = h_puyo[0].x;
            h_puyo[1].y = h_puyo[0].y - 1;
            return true;
        }
        return false;
    } 
    //落下ぷよを固定する
    void fixPuyo(){
        setPuyo(h_puyo[0].x, h_puyo[0].y, new Puyo(h_puyo[0].color));
        setPuyo(h_puyo[1].x, h_puyo[1].y, new Puyo(h_puyo[1].color));
        h_puyo[0].x = -1;
        h_puyo[0].y = -1;
        h_puyo[1].x = -1;
        h_puyo[1].y = -1;
    }
    //消えるぷよを探索する
    void clearCheck(){
        //ステータスを未探索の状態にする
        for(int i=0;i<FIELD_WIDTH * FIELD_HEIGHT;i++)
            if(field[i] != NULL) (*field[i]).status = UNCHECKED;
        bool stillClear = false;
        //探索を開始する
        int counter;
        for(int y=0;y<FIELD_HEIGHT;y++){
            for(int x=0;x<FIELD_WIDTH;x++){
                counter = 0;
                if(getPuyo(x, y) != NULL){
                    count(x, y, &counter, (*getPuyo(x, y)).color);
                    if(counter >= 4){
                        stillClear = true;
                        clear(x, y, (*getPuyo(x, y)).color);
                    }
                }
            }
        }
        view(1, 1);
        if(stillClear == false) return;
        //全てのぷよを下に詰める
        unsigned int delay = 0, ch;
        unsigned int waitCount = 5000;
        bool stillFall = true;
        do{
	        ch = getch();
	        if(delay == 0){
                stillFall = false;
                for(int x=0;x<FIELD_WIDTH;x++){
                    for(int y=FIELD_HEIGHT-2;y>=0;y--){
                        if(getPuyo(x,y+1)==NULL && getPuyo(x,y)!=NULL){
                            for(int k=y;k>=0;k--){
                                field[(k+1) * FIELD_WIDTH + x] = field[k * FIELD_WIDTH + x];
                            }
                            field[x] = NULL;
                            stillFall = true;
                            break;
                        }
                    }
                }
	        }
            view(1,1);
	        delay = (delay + 1) % waitCount;
        }while(stillFall);
        clearCheck();
    }
    //ぷよぷよを再帰的に探す
    void count(int x, int y, int* counter, enum puyocolor color){
        Puyo* puyo = getPuyo(x, y);
        if ((*puyo).status != UNCHECKED) return;
        (*counter)++;
        (*puyo).status = CHECKED;
        
        if(x-1>=0)
            if(getPuyo(x-1, y)!=NULL)
                if((*getPuyo(x-1, y)).color == color)
                    count(x-1, y, counter, color);
        if(x+1<FIELD_WIDTH)
            if(getPuyo(x+1, y)!=NULL)
                if((*getPuyo(x+1, y)).color == color)
                    count(x+1, y, counter, color);
        if(y-1>=0)
            if(getPuyo(x, y-1)!=NULL)
                if((*getPuyo(x, y-1)).color == color)
                    count(x, y-1, counter, color);
        if(y+1<FIELD_HEIGHT)
            if(getPuyo(x, y+1)!=NULL)
                if((*getPuyo(x, y+1)).color == color)
                    count(x, y+1, counter, color);
    }
    //ぷよぷよを消す
    void clear(int x, int y, enum puyocolor color){
        delete field[x + y * FIELD_WIDTH];
        field[x + y * FIELD_WIDTH] = NULL;
        
        if(x-1>=0)
            if(getPuyo(x-1, y)!=NULL)
                if((*getPuyo(x-1, y)).color == color)
                    clear(x-1, y, color);
        if(x+1<FIELD_WIDTH)
            if(getPuyo(x+1, y)!=NULL)
                if((*getPuyo(x+1, y)).color == color)
                    clear(x+1, y, color);
        if(y-1>=0)
            if(getPuyo(x, y-1)!=NULL)
                if((*getPuyo(x, y-1)).color == color)
                    clear(x, y-1, color);
        if(y+1<FIELD_HEIGHT)
            if(getPuyo(x, y+1)!=NULL)
                if((*getPuyo(x, y+1)).color == color)
                    clear(x, y+1, color);
    }
    //フィールドを全消去
    void allClear(){
        for(int i=0;i<FIELD_WIDTH * FIELD_HEIGHT;i++){
            if(field[i] != NULL){
                delete field[i];
                field[i] = NULL;
            }
        }
    }
};
//ゲームエンジン
class PuyoGame{
private:
    //ぷよぷよのフィールド
    Field field;
    //ぷよぷよのスコア
    int score;
public:
    void run(){
        //ぷよを生成する
        field.generate();
        view();

        unsigned int delay = 0;
	    unsigned int waitCount = 100000;
	    int ch, speed_div;
        while(true){
	    //キー受付入力
	    ch = getch();
	    speed_div = 1;

	    switch (ch){
	        case 'Q'://Qの入力で終了
	            return;
	        case KEY_LEFT:
	            field.controllLeft();
		        break;
	        case KEY_RIGHT:
	            field.controllRight();
		        break;
            case 'z':
                field.controllLeftRotate();
            case KEY_DOWN:
                speed_div = 4;
	    }
	    if(delay == 0){
	        if(field.controllDown() == false){
		    field.uncontrollDown();
		    field.fixPuyo();
		    field.clearCheck();
		    field.generate();
	        }
	    }
	    delay ++;
	    if (delay > waitCount / speed_div) delay=0;
	    view();
	}
    }
    void view(){
      //フィールドを表示する
      field.view(1, 1);

      //描写する
      refresh();
    }
    void finish(){
        field.allClear();
    }
};

int main(void){
    initscr();              //画面の初期化
    start_color();          //カラー属性を扱うための初期化
    Puyo::initcolor();        //ぷよの色の設定
    noecho();               //キーを押しても画面に表示しない
    cbreak();               //キー入力を即座に受け付ける
    curs_set(0);
    keypad(stdscr, TRUE);   //キー入力受付方法指定
    timeout(0);             //キー入力非ブロッキングモード
    srand((unsigned int)time(NULL));	//乱数を設定
    
    PuyoGame puyo_game;
    puyo_game.run();
	puyo_game.finish();
    endwin();           //画面をリセット
    return 0;
}
