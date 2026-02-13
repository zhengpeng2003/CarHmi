#include "playhand.h"
#include <QDebug>

PlayHand::PlayHand()
    : _Cards(nullptr), _Type(Hand_Unknown), _Point(Card::Card_begin), _Sheet(0)
{
}

PlayHand::PlayHand(class Cards *cards)
    : _Cards(cards), _Type(Hand_Unknown), _Point(Card::Card_begin), _Sheet(0)
{
    if (cards && !cards->isempty()) {
        classify(cards);
        judeHandType();


    } else {

    }
}

PlayHand::PlayHand(HandType type, Card::cardpoint point, int sheet)
    : _Cards(nullptr), _Type(type), _Point(point), _Sheet(sheet)
{
}

void PlayHand::classify(class Cards *cards)
{

    if (!cards || cards->isempty()) {

        return;
    }

    QVector<Card> Cardlist = cards->Listcardssort();
    int temp[Card::Card_end] = {0}; // 初始化0，避免memset
    memset(temp, 0, sizeof(temp));  // 修复：使用sizeof

    //每次分类都要初始化
    _One_Card.clear();
    _Two_Card.clear();
    _Three_Card.clear();
    _Four_Card.clear();

    for(int i = 0; i < Cardlist.size(); i++)
    {
        Card::cardpoint point = Cardlist[i].getcardpoint();
        if (point >= 0 && point < Card::Card_end) {
            temp[point]++;
        }
    }

    for(int i = Card::Card_3; i < Card::Card_end; i++)  // 从Card_3开始
    {
        int count = temp[i];
        Card::cardpoint currentPoint = (Card::cardpoint)i;

        if(count == 1)
        {
            _One_Card.push_back(currentPoint);
        }
        else if(count == 2)
        {
            _Two_Card.push_back(currentPoint);
        }
        else if(count == 3)
        {
            _Three_Card.push_back(currentPoint);
        }
        else if(count == 4)
        {
            _Four_Card.push_back(currentPoint);
        }
    }


}

void PlayHand::judeHandType()
{
    _Type = Hand_Unknown;
    _Point = Card::Card_begin;
    _Sheet = 0;

    if (_One_Card.size() + _Two_Card.size() + _Three_Card.size() + _Four_Card.size() == 0) {

        return;
    }

    // 按照从大到小的顺序检查牌型
    if (isBombJokers()) {
        _Point = Card::Card_BJ;  // 王炸以大王为点数
        _Sheet = 2;
        _Type = Hand_Bomb_Jokers;

    }
    else if (isBomb()) {
        _Point = _Four_Card[0];
        _Sheet = 4;
        _Type = Hand_Bomb;

    }
    else if (isPlaneTwoPair()) {
        _Point = _Three_Card[0];
        _Sheet = _Three_Card.size() * 5;
        _Type = Hand_Plane_Two_Pair;

    }
    else if (isPlaneTwoSingle()) {
        _Point = _Three_Card[0];
        _Sheet = _Three_Card.size() * 4;
        _Type = Hand_Plane_Two_Single;

    }
    else if (isPlane()) {
        _Point = _Three_Card[0];
        _Sheet = _Three_Card.size() * 3;
        _Type = Hand_Plane;

    }
    else if (isSeqPair()) {
        _Point = _Two_Card[0];
        _Sheet = _Two_Card.size() * 2;
        _Type = Hand_Seq_Pair;

    }
    else if (isSeqSingle()) {
        _Point = _One_Card[0];
        _Sheet = _One_Card.size();
        _Type = Hand_Seq_Sim;

    }
    else if (isTriplePair()) {
        _Point = _Three_Card[0];
        _Sheet = 5;
        _Type = Hand_Triple_Pair;

    }
    else if (isTripleSingle()) {
        _Point = _Three_Card[0];
        _Sheet = 4;
        _Type = Hand_Triple_Single;

    }
    else if (isTriple()) {
        _Point = _Three_Card[0];
        _Sheet = 3;
        _Type = Hand_Triple;

    }
    else if (isPair()) {
        _Point = _Two_Card[0];
        _Sheet = 2;
        _Type = Hand_Pair;

    }
    else if (isSingle()) {
        _Point = _One_Card[0];
        _Sheet = 1;
        _Type = Hand_Single;

    }
    else {

        _Type = Hand_Unknown;
    }
}

bool PlayHand::CanBeat(PlayHand other)
{
    if (this->_Type == Hand_Unknown) return false;
    if (other._Type == Hand_Unknown) return true;

    // 王炸最大
    if (this->_Type == Hand_Bomb_Jokers) return true;

    // 炸弹可以压非炸弹牌型
    if (this->_Type == Hand_Bomb && other._Type != Hand_Bomb && other._Type != Hand_Bomb_Jokers) {
        return true;
    }

    // 同类型牌比较点数（长度必须一致）
    if (this->_Type == other._Type) {
        if (this->_Sheet != other._Sheet) return false;
        return this->_Point > other._Point;
    }

    // 炸弹压炸弹
    if (this->_Type == Hand_Bomb && other._Type == Hand_Bomb) {
        return this->_Point > other._Point;
    }

    return false;
}

bool PlayHand::isSingle(){
    return (_One_Card.size() == 1 && _Two_Card.size() == 0 &&
            _Three_Card.size() == 0 && _Four_Card.size() == 0);
}

bool PlayHand::isPair(){
    return (_One_Card.size() == 0 && _Two_Card.size() == 1 &&
            _Three_Card.size() == 0 && _Four_Card.size() == 0);
}

bool PlayHand::isTriple(){
    return (_One_Card.size() == 0 && _Two_Card.size() == 0 &&
            _Three_Card.size() == 1 && _Four_Card.size() == 0);
}

bool PlayHand::isTripleSingle(){
    const int total = _One_Card.size() + 2 * _Two_Card.size() + 3 * _Three_Card.size() + 4 * _Four_Card.size();
    return (total == 4 && _One_Card.size() == 1 && _Two_Card.size() == 0 &&
            _Three_Card.size() == 1 && _Four_Card.size() == 0);
}

bool PlayHand::isTriplePair(){
    const int total = _One_Card.size() + 2 * _Two_Card.size() + 3 * _Three_Card.size() + 4 * _Four_Card.size();
    return (total == 5 && _One_Card.size() == 0 && _Two_Card.size() == 1 &&
            _Three_Card.size() == 1 && _Four_Card.size() == 0);
}

bool PlayHand::isPlane(){
    if (_Three_Card.size() >= 2 && _One_Card.size() == 0 &&
        _Two_Card.size() == 0 && _Four_Card.size() == 0) {
        // 三顺不能包含2和王
        if (_Three_Card.back() >= Card::Card_2) return false;
        for (int i = 1; i < _Three_Card.size(); i++) {
            if (_Three_Card[i] != _Three_Card[i-1] + 1) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool PlayHand::isPlaneTwoSingle(){
    const int total = _One_Card.size() + 2 * _Two_Card.size() + 3 * _Three_Card.size() + 4 * _Four_Card.size();
    if (_Three_Card.size() >= 2 && _One_Card.size() == _Three_Card.size() &&
        _Two_Card.size() == 0 && _Four_Card.size() == 0 && total == _Three_Card.size() * 4) {
        if (_Three_Card.back() >= Card::Card_2) return false;
        for (int i = 1; i < _Three_Card.size(); i++) {
            if (_Three_Card[i] != _Three_Card[i-1] + 1) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool PlayHand::isPlaneTwoPair(){
    const int total = _One_Card.size() + 2 * _Two_Card.size() + 3 * _Three_Card.size() + 4 * _Four_Card.size();
    if (_Three_Card.size() >= 2 && _Two_Card.size() == _Three_Card.size() &&
        _One_Card.size() == 0 && _Four_Card.size() == 0 && total == _Three_Card.size() * 5) {
        if (_Three_Card.back() >= Card::Card_2) return false;
        for (int i = 1; i < _Three_Card.size(); i++) {
            if (_Three_Card[i] != _Three_Card[i-1] + 1) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool PlayHand::isSeqPair(){
    if (_Two_Card.size() >= 3 && _One_Card.size() == 0 &&
        _Three_Card.size() == 0 && _Four_Card.size() == 0) {
        if (_Two_Card.back() >= Card::Card_2) return false;
        for (int i = 1; i < _Two_Card.size(); i++) {
            if (_Two_Card[i] != _Two_Card[i-1] + 1) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool PlayHand::isSeqSingle(){
    if (_One_Card.size() >= 5 && _Two_Card.size() == 0 &&
        _Three_Card.size() == 0 && _Four_Card.size() == 0) {
        if (_One_Card.back() >= Card::Card_2) return false;
        for (int i = 1; i < _One_Card.size(); i++) {
            if (_One_Card[i] != _One_Card[i-1] + 1) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool PlayHand::isBomb(){
    return (_Four_Card.size() == 1 && _One_Card.size() == 0 &&
            _Two_Card.size() == 0 && _Three_Card.size() == 0);
}

bool PlayHand::isBombJokers(){
    if (_One_Card.size() == 2) {
        bool hasSJ = false, hasBJ = false;
        for (Card::cardpoint point : _One_Card) {
            if (point == Card::Card_SJ) hasSJ = true;
            if (point == Card::Card_BJ) hasBJ = true;
        }
        return hasSJ && hasBJ;
    }
    return false;
}

// 其他方法保持不变...
Cards *PlayHand::Getplayhandcards()
{
    return _Cards;
}

PlayHand::HandType PlayHand::Getplayhandtype()
{
    return _Type;
}

int PlayHand::Getplayhandsheet()
{
    return _Sheet;
}

Card::cardpoint PlayHand::Getplayhandpoint()
{
    return _Point;
}
