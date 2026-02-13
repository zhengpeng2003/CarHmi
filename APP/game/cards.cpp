  #include "cards.h"
#include "qdebug.h"

Cards::Cards(){

}

void Cards::add( Card *card)
{
    _Cards.insert(card);
}
void Cards::add ( Cards * cards )
{
    _Cards.unite(cards->_Cards);
}

void Cards::add(const Cards &cards)
{
    _Cards.unite(cards._Cards);
}

void Cards::add(const Card &card)
{
    _Cards.insert(new Card(card));
}

void Cards::add(const Card *card)
{
    _Cards.insert(new Card(*card));
}
void Cards::remove(Card *card)
{
    _Cards.remove(card);
}
void Cards::remove(Cards* cards) {
    // 基于值而不是指针地址来移除
    QSet<Card*> toRemove;



    for (Card* removeCard : cards->_Cards) {
        // 在手牌中查找相同值的卡片
        auto it = std::find_if(_Cards.begin(), _Cards.end(),
                               [removeCard](Card* myCard) {
                                   return myCard->getcardpoint() == removeCard->getcardpoint() &&
                                          myCard->getcardsuit() == removeCard->getcardsuit();
                               });

        if (it != _Cards.end()) {
            toRemove.insert(*it);
        }
    }


    _Cards.subtract(toRemove);

}
void Cards::remove(QVector<Cards *> cards)
{
    for(int i=0;i<cards.size();i++)
    {
        _Cards.subtract(cards[i]->_Cards);
    }
}

bool Cards::isempty()
{
    return _Cards.isEmpty();
}

void Cards::clearcards()
{
    _Cards.clear();
}

Card::cardpoint Cards::getmaxpoint()
{
    Card::cardpoint max=Card::Card_begin;
    if(!_Cards.isEmpty())
    {
    for(QSet<Card*>::iterator it=_Cards.begin();it!=_Cards.end();++it)
    {
            if((*it)->_Cardpoint>max)
            max=(*it)->_Cardpoint;
    }
    }
    return  max;
}

Card::cardpoint Cards::getminporint()
{
    Card::cardpoint min=Card::Card_end;
    if(!_Cards.isEmpty())
    {
        for(auto it=_Cards.begin();it!=_Cards.end();it++)
        {
            if((*it)->_Cardpoint<min)
                min=(*it)->_Cardpoint;

        }

    }
    return  min;
}

int Cards::GetpointCard(Card::cardpoint point)
{
    int temp=0;
    for(auto i=_Cards.begin();i!=_Cards.end();i++)
    {
        if((*i)->_Cardpoint==point)
            temp++;

    }
    return temp;
}

int Cards::GetCardtotal()
{
    return _Cards.size();
}

bool Cards::contains(Card card)
{
    auto x=_Cards.find(&card);
    if(x!=_Cards.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

Card *Cards::takerandcard()
{

    int num=QRandomGenerator::global()->bounded(_Cards.size());
    QSet<Card*>::iterator it=_Cards.begin();
    for(int i=0;i<num;++i,++it);
    Card *temp_card=*it;
    _Cards.erase(it);
    return temp_card;
}
QListcard Cards::Listcardssort(SortType type)
{

    QListcard list;
    for (Card* card : _Cards) {
        if (card) {
            list.append(*card);  // 需要Card有拷贝构造函数
        }
    }
    if(type==ASC)
    {
        std::sort( list.begin(), list.end(),lesssort);
        return list;
    }
    else if(type==DESC)
    {
        std::sort(list.begin(), list.end(),greaterort);
        return list;
    }
    else
        return list;

}
