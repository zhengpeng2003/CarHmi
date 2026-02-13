#ifndef STRATEGY_H
#define STRATEGY_H

#include "player.h"
#include "cards.h"
#include "playhand.h"
#include <QVector>

struct Info
{
    bool beat;
    Card::cardpoint begin;
    Card::cardpoint end;
    int number;
    int base;
};

class Strategy
{
public:
    Strategy(player* player, Cards& cards);
    Cards* GetrootPlayhand();


    Cards* FirstPlayhand();
    bool IsPlayHand(PlayHand type);
    Cards* Getbigplayhand(PlayHand type);

    // 牌力评估函数
    int EvaluateHandStrength(const Cards& cards);
    bool ShouldPressCard(const PlayHand& opponentHand);
    int EvaluateCardValue(Cards* cards);
    bool ShouldPassSmallCards();

    Cards* Getsamepointcard(Card::cardpoint point, int count);
    QVector<Cards*> Getsamecount(int count);
    Cards* GetrangCard(Card::cardpoint begin, Card::cardpoint end);
    QVector<Cards*> GetCardstype(PlayHand playhand, bool beat);

    QVector<Cards*> FindHand_Sing123(Card::cardpoint point, int count);
    QVector<Cards*> FindHand_Triple12(Card::cardpoint beginPoint, int count);
    QVector<Cards*> FindHand_Plane12(Card::cardpoint beginPoint, int attachCount);
    Cards* Getpair(Card::cardpoint point);
    Cards* Getsim(Card::cardpoint point);
    QVector<Cards*> FindHand_parisim(Info info);

    QVector<Cards> findCardsByCount(int count);
    QVector<Cards*> findCardsByCountVec(int count);
    Cards findSamePointCardsValue(Card::cardpoint point, int count);

private:
    player* m_player;
    Cards m_cards;
};

#endif // STRATEGY_H
