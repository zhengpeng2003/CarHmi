#ifndef CARD_H
#define CARD_H
#include <QVector>
class Card
{
public:
    friend class Cards;
    Card();
    enum cardpoint
    {
        Card_begin=0,
        Card_3,
        Card_4,
        Card_5,
        Card_6,
        Card_7,
        Card_8,
        Card_9,
        Card_10,
        Card_J,
        Card_Q,
        Card_K,
        Card_A,
        Card_2,
        Card_SJ,
        Card_BJ,
        Card_end,

    };
    enum cardsuit
    {
        Suit_Begin=0,
        Heart,
        Diamond,
        Club,
        Spade,
        Suit_End
    };
    Card(cardsuit Cardsuit,cardpoint Cardpoint);

    cardpoint getcardpoint() const;
    cardsuit getcardsuit()const;

    void setcardporint(cardpoint Cardpoint);
    void setcardsuit(cardsuit Cardsuit);
    void Carddelete();
private:
    cardpoint _Cardpoint;
    cardsuit _Cardsuit;
};

using QListcard=QVector<Card>;
bool lesssort(const Card L,const Card R);
bool greaterort(const Card L,const Card R);
bool operator==(const Card &L,const Card& R);
bool operator<(const Card &L,const Card &R);
size_t qHash(const Card &temp,size_t seed);
#endif // CARD_H
