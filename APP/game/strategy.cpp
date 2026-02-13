#include "strategy.h"
#include <QDebug>
#include <limits>

Strategy::Strategy(player *player, Cards &cards)
    : m_player(player), m_cards(cards)
{
}

Cards* Strategy::GetrootPlayhand()
{
    player* preplayer = m_player ? m_player->GetPendPlayer() : nullptr;
    Cards* preCards = m_player ? m_player->GetPendCard() : nullptr;

    if (preplayer == nullptr || preplayer == m_player) {
        return FirstPlayhand();
    } else {
        PlayHand type(preCards);
        bool canPlay = IsPlayHand(type);
        if (canPlay) {
            return Getbigplayhand(type);
        } else {
            return new Cards();
        }
    }
}

Cards* Strategy::FirstPlayhand()
{
    auto validatePlay = [](Cards* cards) -> Cards* {
        if (!cards) return nullptr;
        PlayHand ph(cards);
        if (ph.Getplayhandtype() == PlayHand::Hand_Unknown) {
            delete cards;
            return nullptr;
        }
        return cards;
    };

    auto selectBest = [this](QVector<Cards*> &options) -> Cards* {
        if (options.isEmpty()) return nullptr;

        int bestScore = std::numeric_limits<int>::min();
        int bestIdx = 0;
        for (int i = 0; i < options.size(); ++i) {
            int score = EvaluateCardValue(options[i]);
            if (score > bestScore) {
                bestScore = score;
                bestIdx = i;
            }
        }

        Cards* chosen = options[bestIdx];
        for (int i = 0; i < options.size(); ++i) {
            if (i != bestIdx) {
                delete options[i];
            }
        }
        return chosen;
    };

    // 1. 顺子/连对等长连牌优先，能快速减少手牌
    Info seqInfo;
    seqInfo.beat = false;
    seqInfo.begin = Card::Card_3;
    seqInfo.end = Card::Card_10;
    seqInfo.number = 1;
    seqInfo.base = 5;
    QVector<Cards*> seqResults = FindHand_parisim(seqInfo);
    if (!seqResults.isEmpty()) {
        Cards* result = validatePlay(selectBest(seqResults));
        if (result) return result;
    }

    PlayHand seqPairPH(PlayHand::Hand_Seq_Pair, Card::Card_begin, 0);
    QVector<Cards*> seqPairCandidates = GetCardstype(seqPairPH, false);
    if (!seqPairCandidates.isEmpty()) {
        Cards* result = validatePlay(selectBest(seqPairCandidates));
        if (result) return result;
    }

    // 2. 飞机、三带类，尽量以组合方式出掉中小牌
    QVector<Cards*> planeWithWings = FindHand_Plane12(Card::Card_3, 2);
    if (planeWithWings.isEmpty()) {
        planeWithWings = FindHand_Plane12(Card::Card_3, 1);
    }
    if (planeWithWings.isEmpty()) {
        PlayHand planePH(PlayHand::Hand_Plane, Card::Card_begin, 0);
        planeWithWings = GetCardstype(planePH, false);
    }
    if (!planeWithWings.isEmpty()) {
        Cards* result = validatePlay(selectBest(planeWithWings));
        if (result) return result;
    }

    QVector<Cards*> tripleWith = FindHand_Triple12(Card::Card_3, 2);
    if (tripleWith.isEmpty()) tripleWith = FindHand_Triple12(Card::Card_3, 1);
    if (!tripleWith.isEmpty()) {
        Cards* result = validatePlay(selectBest(tripleWith));
        if (result) return result;
    }

    PlayHand triPH(PlayHand::Hand_Triple, Card::Card_begin, 0);
    QVector<Cards*> tripleCandidates = GetCardstype(triPH, false);
    if (!tripleCandidates.isEmpty()) {
        Cards* result = validatePlay(selectBest(tripleCandidates));
        if (result) return result;
    }

    // 3. 对子优先于裸单，避免留下大量散牌
    QVector<Cards*> pairs = FindHand_Sing123(Card::Card_3, 2);
    if (!pairs.isEmpty()) {
        Cards* result = validatePlay(selectBest(pairs));
        if (result) return result;
    }

    // 4. 尝试小单牌
    QVector<Cards*> singles = FindHand_Sing123(Card::Card_3, 1);
    if (!singles.isEmpty()) {
        Cards* result = validatePlay(selectBest(singles));
        if (result) return result;
    }

    // 5. 最后才会考虑炸弹
    QVector<Cards*> bombs = Getsamecount(4);
    if (!bombs.isEmpty()) {
        Cards* result = validatePlay(selectBest(bombs));
        if (result) return result;
    }

    return new Cards();
}

bool Strategy::IsPlayHand(PlayHand type)
{
    player* preplayer = m_player ? m_player->GetPendPlayer() : nullptr;
    Cards* preCards = m_player ? m_player->GetPendCard() : nullptr;

    if (!preplayer || !preCards) return false;

    const bool sameRole = (preplayer->GetRole() == m_player->GetRole());
    const bool isFarmerTeam = sameRole && m_player->GetRole() == player::FORMAR;
    QVector<Cards*> list = GetCardstype(type, true);
    const bool hasOption = !list.isEmpty();

    if (!hasOption) {
        for (auto p : list) delete p;
        return false;
    }

    if (!sameRole) {
        for (auto p : list) delete p;
        return true;
    }

    bool canFinish = false;
    bool hasPowerCombo = false;
    const int myHandCount = m_cards.GetCardtotal();
    const bool teammateJustPlayed = sameRole;
    const bool iHavePlentyCards = myHandCount >= 10;
    const bool teammateStillHealthy = preplayer->GetCards().GetCardtotal() > 4;
    for (auto option : list) {
        if (option->GetCardtotal() == m_cards.GetCardtotal()) {
            canFinish = true;
        }
        PlayHand optionType(option);
        if (optionType.Getplayhandtype() == PlayHand::Hand_Bomb) {
            hasPowerCombo = true;
        }
    }

    const bool lowHand = myHandCount <= 5;
    const bool teammateIsCritical = preplayer->GetCards().GetCardtotal() <= 2;
    const bool shouldHelpTeammate = isFarmerTeam && teammateJustPlayed && iHavePlentyCards && teammateStillHealthy;

    for (auto p : list) delete p;

    return canFinish || lowHand || teammateIsCritical || hasPowerCombo || shouldHelpTeammate;
}

Cards* Strategy::Getbigplayhand(PlayHand type)
{
    player* preplayer = m_player ? m_player->GetPendPlayer() : nullptr;
    if (!preplayer) return new Cards();

    const bool sameRole = (preplayer->GetRole() == m_player->GetRole());

    if (!sameRole && preplayer->GetCards().GetCardtotal() <= 3) {
        QVector<Cards*> bombs = Getsamecount(4);
        for (auto b : bombs) {
            PlayHand bph(b);
            if (bph.CanBeat(type)) {
                Cards* result = b;
                for (auto bomb : bombs) {
                    if (bomb != b) {
                        delete bomb;
                    }
                }
                return result;
            }
        }
        for (auto p : bombs) delete p;

        Cards* sj = Getsamepointcard(Card::Card_SJ, 1);
        Cards* bj = Getsamepointcard(Card::Card_BJ, 1);
        if (!sj->isempty() && !bj->isempty()) {
            Cards* jokers = new Cards();
            for (auto card : sj->Listcardssort()) jokers->add(card);
            for (auto card : bj->Listcardssort()) jokers->add(card);
            delete sj;
            delete bj;
            return jokers;
        }
        delete sj;
        delete bj;
        return new Cards();
    }

    QVector<Cards*> candidates = GetCardstype(type, true);
    if (!candidates.isEmpty()) {
        player* nextplayer = m_player->GetNextPlayer();
        Cards* result = nullptr;

        auto scoreBeat = [this, sameRole](Cards* option) {
            PlayHand ph(option);
            // 越小越好，避免浪费大牌；拆炸弹、双王会有额外惩罚
            int base = static_cast<int>(ph.Getplayhandpoint());
            int bombPenalty = 0;
            for (auto card : option->Listcardssort()) {
                if (m_cards.GetpointCard(card.getcardpoint()) == 4) {
                    bombPenalty += 6;
                }
            }

            // 出完这一手后的剩余牌形越整齐越好
            Cards remain = m_cards;
            remain.remove(option);
            Strategy remainSt(m_player, remain);
            int remainScore = remainSt.EvaluateCardValue(&remain);

            const int myRemainCount = remain.GetCardtotal();
            int partnerPenalty = sameRole ? 2 : 0;
            int aggressionBonus = sameRole ? 0 : 2;

            if (sameRole && m_player && m_player->GetRole() == player::FORMAR) {
                if (myRemainCount >= 10 || m_cards.GetCardtotal() >= 12) {
                    partnerPenalty = 0;              // 牌多时适当放宽
                    aggressionBonus += 1;            // 更主动压队友牌
                }
            }

            return -base - bombPenalty - partnerPenalty + aggressionBonus + option->GetCardtotal() + remainScore / 10;
        };

        int bestScore = std::numeric_limits<int>::min();
        int bestIdx = 0;
        for (int i = 0; i < candidates.size(); ++i) {
            int score = scoreBeat(candidates[i]);
            // 若下家要走牌，适当提高能直接压住的高分组合
            if (nextplayer && nextplayer->GetRole() != m_player->GetRole() && nextplayer->GetCards().GetCardtotal() <= 2) {
                score += candidates[i]->GetCardtotal();
            }
            if (score > bestScore) {
                bestScore = score;
                bestIdx = i;
            }
        }

        result = candidates[bestIdx];
        for (int i = 0; i < candidates.size(); ++i) {
            if (i != bestIdx) {
                delete candidates[i];
            }
        }
        PlayHand validate(result);
        if (validate.Getplayhandtype() != PlayHand::Hand_Unknown) {
            return result;
        }
        delete result;
        return new Cards();
    }

    QVector<Cards*> bombs2 = Getsamecount(4);
    for (auto b : bombs2) {
        PlayHand bph(b);
        if (bph.CanBeat(type)) {
            Cards* result = b;
            for (auto bomb : bombs2) {
                if (bomb != b) {
                    delete bomb;
                }
            }
            return result;
        }
    }
    for (auto p : bombs2) delete p;

    return new Cards();
}

Cards* Strategy::Getsamepointcard(Card::cardpoint point, int count)
{
    Cards* tempcards = new Cards();
    if (count < 1 || count > 4) return tempcards;

    if (point == Card::Card_SJ || point == Card::Card_BJ) {
        for (auto c : m_cards.Listcardssort()) {
            if (c.getcardpoint() == point) {
                tempcards->add(c);
                return tempcards;
            }
        }
        return tempcards;
    }

    int found = 0;
    for (auto c : m_cards.Listcardssort()) {
        if (c.getcardpoint() == point) {
            tempcards->add(c);
            ++found;
            if (found == count) break;
        }
    }
    if (found != count) {
        tempcards->clearcards();
    }
    return tempcards;
}

QVector<Cards*> Strategy::Getsamecount(int count)
{
    QVector<Cards*> out;
    if (count < 1 || count > 4) return out;

    for (int i = Card::Card_begin + 1; i < Card::Card_end; ++i) {
        Card::cardpoint point = Card::cardpoint(i);
        if (m_cards.GetpointCard(point) >= count) {
            Cards* cs = Getsamepointcard(point, count);
            if (!cs->isempty()) {
                out.append(cs);
            } else {
                delete cs;
            }
        }
    }
    return out;
}

Cards* Strategy::GetrangCard(Card::cardpoint begin, Card::cardpoint end)
{
    Cards* res = new Cards();
    for (int p = begin; p < end; ++p) {
        Card::cardpoint point = Card::cardpoint(p);
        int cnt = m_cards.GetpointCard(point);
        if (cnt > 0) {
            Cards tmp = findSamePointCardsValue(point, cnt);
            for (auto card : tmp.Listcardssort()) {
                res->add(card);
            }
        }
    }
    return res;
}

QVector<Cards*> Strategy::GetCardstype(PlayHand playhand, bool beat)
{
    QVector<Cards*> result;
    PlayHand::HandType t = playhand.Getplayhandtype();
    Card::cardpoint pt = playhand.Getplayhandpoint();
    Card::cardpoint start = beat ? Card::cardpoint(pt + 1) : Card::Card_3;

    switch (t) {
    case PlayHand::Hand_Single:
        result = FindHand_Sing123(start, 1);
        break;
    case PlayHand::Hand_Pair:
        result = FindHand_Sing123(start, 2);
        break;
    case PlayHand::Hand_Triple:
        result = FindHand_Sing123(start, 3);
        break;
    case PlayHand::Hand_Triple_Single:
        result = FindHand_Triple12(start, 1);
        break;
    case PlayHand::Hand_Triple_Pair:
        result = FindHand_Triple12(start, 2);
        break;
    case PlayHand::Hand_Plane:
        result = FindHand_Plane12(start, 0);
        break;
    case PlayHand::Hand_Plane_Two_Single:
        result = FindHand_Plane12(start, 1);
        break;
    case PlayHand::Hand_Plane_Two_Pair:
        result = FindHand_Plane12(start, 2);
        break;
    case PlayHand::Hand_Seq_Pair:
    {
        Info info;
        info.base = beat ? playhand.Getplayhandsheet() / 2 : 3;
        info.beat = beat;
        info.begin = Card::Card_3;
        info.end = Card::Card_Q;
        info.number = 2;
        result = FindHand_parisim(info);
        break;
    }
    case PlayHand::Hand_Seq_Sim:
    {
        Info info;
        info.base = beat ? playhand.Getplayhandsheet() : 5;
        info.beat = beat;
        info.begin = Card::Card_3;
        info.end = Card::Card_10;
        info.number = 1;
        result = FindHand_parisim(info);
        break;
    }
    case PlayHand::Hand_Bomb:
        result = Getsamecount(4);
        break;
    default:
        return result;
    }

    if (beat) {
        const int targetSheet = playhand.Getplayhandsheet();
        for (int i = result.size() - 1; i >= 0; --i) {
            if (result[i]->GetCardtotal() != targetSheet) {
                delete result[i];
                result.remove(i);
            }
        }
    }

    for (int i = result.size() - 1; i >= 0; --i) {
        PlayHand candidateType(result[i]);
        if (candidateType.Getplayhandtype() != t) {
            delete result[i];
            result.remove(i);
        }
    }

    return result;
}

QVector<Cards*> Strategy::FindHand_Sing123(Card::cardpoint point, int count)
{
    QVector<Cards*> result;
    if (m_cards.isempty()) return result;

    for (int p = point; p < Card::Card_end; ++p) {
        Card::cardpoint currentPoint = Card::cardpoint(p);
        if (m_cards.GetpointCard(currentPoint) >= count) {
            Cards* cs = Getsamepointcard(currentPoint, count);
            if (!cs->isempty()) {
                result.append(cs);
            } else {
                delete cs;
            }
        }
    }
    return result;
}

QVector<Cards*> Strategy::FindHand_Triple12(Card::cardpoint beginPoint, int count)
{
    QVector<Cards*> result;
    if (m_cards.isempty()) return result;

    QVector<Card::cardpoint> triplePoints;
    for (int p = Card::Card_3; p < Card::Card_end; ++p) {
        Card::cardpoint point = Card::cardpoint(p);
        if (m_cards.GetpointCard(point) >= 3) {
            triplePoints.append(point);
        }
    }

    for (Card::cardpoint tp : triplePoints) {
        if (tp < beginPoint) continue;

        Cards* three = Getsamepointcard(tp, 3);
        if (three->isempty()) {
            delete three;
            continue;
        }

        if (count == 0) {
            result.append(three);
            continue;
        }

        Cards remain = m_cards;
        remain.remove(three);
        Strategy remainStrategy(m_player, remain);

        QVector<Cards*> attaches = remainStrategy.FindHand_Sing123(Card::Card_3, count);
        Cards* attach = nullptr;
        for (auto a : attaches) {
            if (!a->isempty()) {
                attach = a;
                break;
            }
        }

        if (attach) {
            for (auto card : attach->Listcardssort()) {
                three->add(card);
            }
            result.append(three);
        } else {
            delete three;
        }

        for (auto a : attaches) {
            if (a != attach) delete a;
        }
        if (attach) delete attach;
    }
    return result;
}

QVector<Cards*> Strategy::FindHand_Plane12(Card::cardpoint beginPoint, int attachCount)
{
    QVector<Cards*> result;
    if (m_cards.isempty()) return result;

    for (int p = beginPoint; p <= Card::Card_A; ++p) {
        Card::cardpoint point1 = Card::cardpoint(p);
        Card::cardpoint point2 = Card::cardpoint(p + 1);

        Cards* t1 = Getsamepointcard(point1, 3);
        Cards* t2 = Getsamepointcard(point2, 3);

        if (t1->isempty() || t2->isempty()) {
            delete t1;
            delete t2;
            continue;
        }

        Cards* plane = new Cards();
        for (auto card : t1->Listcardssort()) plane->add(card);
        for (auto card : t2->Listcardssort()) plane->add(card);

        bool validAttach = true;

        if (attachCount > 0) {
            Cards remain = m_cards;
            remain.remove(plane);

            Strategy remainStrategy(m_player, remain);
            QVector<Cards*> attaches = remainStrategy.FindHand_Sing123(Card::Card_3, attachCount);
            const int neededAttach = 2; // 当前仅支持2连飞机

            if (attaches.size() >= neededAttach) {
                for (int i = 0; i < neededAttach; ++i) {
                    if (!attaches[i]->isempty()) {
                        for (auto card : attaches[i]->Listcardssort()) {
                            plane->add(card);
                        }
                    }
                }
            } else {
                validAttach = false;
            }

            for (auto a : attaches) delete a;
        }

        if (validAttach) {
            result.append(plane);
        } else {
            delete plane;
        }
        delete t1;
        delete t2;
    }
    return result;
}

Cards* Strategy::Getpair(Card::cardpoint point)
{
    Cards* seq = new Cards();
    bool found = false;

    for (int p = Card::Card_3; p <= Card::Card_Q; ++p) {
        Card::cardpoint point1 = Card::cardpoint(p);
        Card::cardpoint point2 = Card::cardpoint(p + 1);
        Card::cardpoint point3 = Card::cardpoint(p + 2);

        Cards* t1 = Getsamepointcard(point1, 2);
        Cards* t2 = Getsamepointcard(point2, 2);
        Cards* t3 = Getsamepointcard(point3, 2);

        if (!t1->isempty() && !t2->isempty() && !t3->isempty()) {
            for (auto card : t1->Listcardssort()) seq->add(card);
            for (auto card : t2->Listcardssort()) seq->add(card);
            for (auto card : t3->Listcardssort()) seq->add(card);
            found = true;
        }

        delete t1;
        delete t2;
        delete t3;

        if (found) break;
    }

    if (!found) {
        delete seq;
        return new Cards();
    }
    return seq;
}

Cards* Strategy::Getsim(Card::cardpoint point)
{
    Q_UNUSED(point)
    Cards* seq = new Cards();
    bool found = false;

    for (int p = Card::Card_3; p <= Card::Card_10; ++p) {
        Card::cardpoint point1 = Card::cardpoint(p);
        Card::cardpoint point2 = Card::cardpoint(p + 1);
        Card::cardpoint point3 = Card::cardpoint(p + 2);

        Cards* t1 = Getsamepointcard(point1, 1);
        Cards* t2 = Getsamepointcard(point2, 1);
        Cards* t3 = Getsamepointcard(point3, 1);

        if (!t1->isempty() && !t2->isempty() && !t3->isempty()) {
            for (auto card : t1->Listcardssort()) seq->add(card);
            for (auto card : t2->Listcardssort()) seq->add(card);
            for (auto card : t3->Listcardssort()) seq->add(card);
            found = true;
        }

        delete t1;
        delete t2;
        delete t3;

        if (found) break;
    }

    if (!found) {
        delete seq;
        return new Cards();
    }
    return seq;
}

QVector<Cards*> Strategy::FindHand_parisim(Info info)
{
    QVector<Cards*> result;
    if (m_cards.isempty()) return result;

    for (int p = info.begin; p <= info.end; ++p) {
        bool ok = true;
        Cards* seq = new Cards();

        for (int k = 0; k < info.base; ++k) {
            Card::cardpoint currentPoint = Card::cardpoint(p + k);
            if (currentPoint >= Card::Card_2) {
                ok = false;
                break;
            }

            Cards* cs = Getsamepointcard(currentPoint, info.number);
            if (cs->isempty()) {
                ok = false;
                delete cs;
                break;
            } else {
                for (auto card : cs->Listcardssort()) {
                    seq->add(card);
                }
                delete cs;
            }
        }

        if (ok) {
            result.append(seq);
        } else {
            delete seq;
        }
    }
    return result;
}

QVector<Cards> Strategy::findCardsByCount(int count)
{
    QVector<Cards> arr;
    if (count < 1 || count > 4) return arr;

    for (int p = Card::Card_3; p < Card::Card_end; ++p) {
        Card::cardpoint point = Card::cardpoint(p);
        if (m_cards.GetpointCard(point) == count) {
            Cards cs = findSamePointCardsValue(point, count);
            if (!cs.isempty()) {
                arr.append(cs);
            }
        }
    }
    return arr;
}

QVector<Cards*> Strategy::findCardsByCountVec(int count)
{
    QVector<Cards*> out;
    if (count < 1 || count > 4) return out;

    for (int p = Card::Card_3; p < Card::Card_end; ++p) {
        Card::cardpoint point = Card::cardpoint(p);
        if (m_cards.GetpointCard(point) == count) {
            Cards* cs = Getsamepointcard(point, count);
            if (!cs->isempty()) {
                out.append(cs);
            } else {
                delete cs;
            }
        }
    }
    return out;
}

Cards Strategy::findSamePointCardsValue(Card::cardpoint point, int count)
{
    Cards cs;
    if (count < 1 || count > 4) return cs;

    if (point == Card::Card_SJ || point == Card::Card_BJ) {
        if (count != 1) return cs;
        for (auto card : m_cards.Listcardssort()) {
            if (card.getcardpoint() == point) {
                cs.add(card);
                return cs;
            }
        }
        return cs;
    }

    int found = 0;
    for (auto card : m_cards.Listcardssort()) {
        if (card.getcardpoint() == point) {
            cs.add(card);
            ++found;
            if (found == count) break;
        }
    }
    if (found != count) {
        cs.clearcards();
    }
    return cs;
}

int Strategy::EvaluateCardValue(Cards *cards)
{
    if (!cards) return std::numeric_limits<int>::min();

    PlayHand ph(cards);
    int cardCount = cards->GetCardtotal();
    int pointValue = static_cast<int>(ph.Getplayhandpoint());

    // 避免轻易拆炸弹、双王，使用惩罚项
    int structurePenalty = 0;
    for (auto card : cards->Listcardssort()) {
        if (m_cards.GetpointCard(card.getcardpoint()) == 4) {
            structurePenalty += 6;
        }
        if (card.getcardpoint() == Card::Card_SJ || card.getcardpoint() == Card::Card_BJ) {
            structurePenalty += 3;
        }
    }

    // 越多张、点数越小越优先
    return cardCount * 4 - pointValue - structurePenalty;
}

int Strategy::EvaluateHandStrength(const Cards &cards)
{
    Cards temp(cards);
    Strategy tempSt(m_player, temp);
    return tempSt.EvaluateCardValue(&temp);
}

bool Strategy::ShouldPassSmallCards()
{
    // 在没有有效组合时才会退让小牌
    return m_cards.GetCardtotal() > 10;
}

bool Strategy::ShouldPressCard(const PlayHand &opponentHand)
{
    Q_UNUSED(opponentHand);
    // 这里仅做轻量级策略，后续可扩展
    return true;
}
