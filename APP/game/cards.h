#ifndef CARDS_H
#define CARDS_H
#include "card.h"
#include <QSet>
#include <QRandomGenerator>
class Cards
{
public:
    enum SortType
    {
        ASC,
        DESC,
        NOSORT,
    };
    Cards();
    //添加一张牌
    void add(Card *card);
    void add(const Card& card);
    void add(const Card *card);
    //添加多张牌
    void add (Cards *cards );
    void add(const Cards& cards);
    void add (const Cards *cards );
    //出一张牌


    //添加多张牌
    void remove(Card *card);
    //出多张牌
    void remove(Cards*cards);
    void remove(QVector<Cards*> cards);
    //判断牌是否为空
    bool isempty();
    //清空牌
    void clearcards();
    //获取当前最大点数
    Card::cardpoint getmaxpoint();
    //获取当前最小点数
    Card::cardpoint getminporint();
    //指定点数牌的数量
    int GetpointCard(Card::cardpoint point);
    //获取卡牌的数量
    int GetCardtotal();

    //某张牌是否在集合中
    bool contains(Card card);

    //发牌函数
    Card *takerandcard();
    //给牌排序
   QListcard  Listcardssort(SortType type = ASC);
private:
    QSet<Card*>_Cards;

};


#endif // CARDS_H
