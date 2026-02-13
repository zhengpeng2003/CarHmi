#include "card.h"
Card::Card()
{

}
Card::Card(cardsuit Cardsuit, cardpoint Cardpoint)
{
    this->_Cardpoint=Cardpoint;
    this->_Cardsuit=Cardsuit;
}
Card::cardpoint Card::getcardpoint()const
{
    return _Cardpoint;
}
Card::cardsuit Card::getcardsuit()const
{
    return _Cardsuit;
};
void Card::setcardporint(cardpoint Cardpoint)
{
    this->_Cardpoint=Cardpoint;
}
void Card::setcardsuit(cardsuit Cardsuit)
{
    this->_Cardsuit=Cardsuit;
}

void Card::Carddelete()
{
    delete this;
}

bool lesssort(const Card L, const Card R)
{

    if(L.getcardpoint()<R.getcardpoint())
    {
        return true;
    }
    else if(L.getcardpoint()==R.getcardpoint())
    {
        if(L.getcardsuit()<R.getcardsuit())
            return false;
        else
            return true;
    }
    else
    {
        return false;

    }
}

bool greaterort(const Card L, const Card R)
{
    if(L.getcardpoint()>R.getcardpoint())
    {
        return true;
    }
    else if(L.getcardpoint()==R.getcardpoint())
    {
        if(L.getcardsuit()<R.getcardsuit())
            return false;
        else
            return true;
    }
    else
    {
        return false;

    }
}
bool operator==(const Card& L, const Card& R) {
    return L.getcardpoint() == R.getcardpoint() && L.getcardsuit() == R.getcardsuit();
}
inline size_t qHash(const Card& card, size_t seed = 0) {
    return qHash(card.getcardpoint(), seed) ^ qHash(card.getcardsuit(), seed + 1);
}
bool operator<(const Card& L, const Card& R) {
    if (L.getcardpoint() != R.getcardpoint())
        return L.getcardpoint() < R.getcardpoint();  // 先比较点数
    return L.getcardsuit() < R.getcardsuit();       // 再比较花色
}
