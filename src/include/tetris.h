#ifndef SRC_TETRIS_H_
#define SRC_TETRIS_H_

class Board;

class Tetris{
public:
    //状态: 未使用, 待命, 下落中, 着陆
    enum Status {INACTIVE, WAITING, FALLING, LANDED};
    Status status;
    //移动状态: 未移动(左右), 左移, 右移
    enum Movement {NONE = 0, LEFT = -1, RIGHT = 1};
    Movement movement;

    static const int SIZE = 4; //方块边界
    //共有七个方块型(Z,J,O,T,S,I,L),每个型有四个方块, 分别存放四个方块的相对坐标
    static const int block_table[7][4][2];

    explicit Tetris(int n_type);

    //改变坐标//
    void setPoint(int xx, int yy){x = xx; y = yy;}

    void setX(int i, int new_x){x = new_x - relateLocate[i][0];}
    void setY(int i, int new_y){y = new_y - relateLocate[i][1];}
    //获取坐标//
    int getX(int i) const{return x + relateLocate[i][0];}
    int getY(int i) const{return y + relateLocate[i][1];}

    void add_to_x(int x_offset){x += x_offset;}


    //方块操作
    void rotateRight() const; //右转
    void rotateLeft() const;  //左转

    void getShadow(Board *board, int shadow_y[]);   //方块降落预览

    int x{}, y{};               //方块型相对位置中(0,0)的全局坐标
    int (*relateLocate)[2];  //相对中心方块坐标
    int type;               //方块种类(0-6)

    bool has_landed() const{return status == LANDED;}
    void lands() {status = LANDED;}
    void drop()  {status = FALLING;}

    ////开关////
    bool fall;      //是否快速下落
    bool speedup;   //是否加速
    bool shift{};     //左转(true)&&k,右转(false)
    bool rotate{};    //是否旋转
};
#endif
