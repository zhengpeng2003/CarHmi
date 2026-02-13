#ifndef PLAYHAND_H
#define PLAYHAND_H
#include "card.h"
#include "cards.h"

class PlayHand
{

public:
    enum HandType {
        // 基础牌型
        Hand_Unknown,          //不出
        Hand_Single,           // 单
        Hand_Pair,             // 对
        Hand_Triple,           // 三个
        Hand_Triple_Single,    // 三带一
        Hand_Triple_Pair,      // 三带对

        // 连牌牌型
        Hand_Plane,            // 飞机
        Hand_Plane_Two_Single, // 飞机带两单
        Hand_Plane_Two_Pair,   // 飞机带两对
        Hand_Seq_Pair,         // 连对
        Hand_Seq_Sim,          // 顺子
        // 炸弹牌型
        Hand_Bomb,
        Hand_Bomb_Jokers             // 炸弹

    };
    PlayHand();
    explicit PlayHand( Cards * cards);
    PlayHand(PlayHand::HandType type,Card::cardpoint point,int sheet);
    void classify(Cards * cards);//用来存储打出的牌的point各有几张
    void judeHandType();//判断打出来牌的类型进行存储
    bool CanBeat( PlayHand  other);//绝斗
    // 基础牌型判断
    bool isSingle();           // 单
    bool isPair();             // 对
    bool isTriple();           // 三个(相同)
    bool isTripleSingle();     // 三带一
    bool isTriplePair();       // 三带二

    // 连牌牌型判断
    bool isPlane();            // 飞机
    bool isPlaneTwoSingle();   // 飞机带两单
    bool isPlaneTwoPair();     // 飞机带2对
    bool isSeqPair();           // 连对
    bool isSeqSingle();         // 顺子

    // 炸弹牌型判断
    bool isBomb();             // 炸弹
    bool isBombSingle();       // 炸弹带一单
    bool isBombPair();         // 炸弹带一对
    bool isBombTwoSingle();    // 炸弹带两单

    // 王炸牌型判断
    bool isBombJokers();       // 王炸
    bool isBombJokersSingle(); // 王炸带一单
    bool isBombJokersPair();   // 王炸带一对
    bool isBombJokersTwoSingle(); // 王炸带两单

    Cards * Getplayhandcards();
    PlayHand::HandType Getplayhandtype();
    int Getplayhandsheet();
    Card::cardpoint Getplayhandpoint();
private:
    Cards * _Cards;//卡牌
    PlayHand::HandType _Type;//卡牌类型
    Card::cardpoint _Point;//点数
    int _Sheet;//张数
    //牌的类型
    QVector<Card::cardpoint> _One_Card;//point 只有1张
    QVector<Card::cardpoint> _Two_Card;//point 只有2张
    QVector<Card::cardpoint> _Three_Card;//point 只有3张
    QVector<Card::cardpoint> _Four_Card;//point 只有4张


};


#endif // PLAYHAND_H
