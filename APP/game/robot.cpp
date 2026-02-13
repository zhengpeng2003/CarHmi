#include "robot.h"

void robot::PrepareGetLord()
{
    Robotgetloard* T1=new Robotgetloard(this,this);
    T1->start();
}

void robot::PreparePlayCard()
{
    RobotPlayhand * T2=new RobotPlayhand(this,this);
    T2->start();
}

void robot::RobotGetLard()
{

    //     *基于手中的牌计算权重
    //     *大小王:6
    //     *顺子/炸弹:5
    //     *三张点数相同的牌:4
    //     *2的权重:3
    //     *对儿牌:1
    int Totalgrade=0;
    Cards store(_Cards);//创建临时值后面方便移动
    Strategy st(this,store);
    Cards *temp=st.GetrangCard(Card::Card_SJ,Card::Card_BJ);//大小王
    Totalgrade+=temp->GetCardtotal()*6;
    store.remove(temp);

    Cards * temp1=st.Getsim(store.getminporint());//顺子/炸弹:5
    Totalgrade+=temp1->GetCardtotal()*5;
    store.remove(temp1);

    Cards * temp2=st.Getpair(Card::Card_2);//*2的权重:3
    Totalgrade+=temp2->GetCardtotal()*3;
    store.remove(temp2);

    QVector<Cards*>  temp3=st.Getsamecount(2);//*对儿牌:1
    Totalgrade+=temp3.size()*1;
    store.remove(temp3);

    QVector<Cards*>  temp4=st.Getsamecount(4);//炸弹:5
    Totalgrade+=temp4.size()*4;
    store.remove(temp4);

    if(Totalgrade >22)
    {
        grablordbet(3);
    }
    else if(Totalgrade <=22 && Totalgrade >18)
    {
        grablordbet(2);
    }
    else if(Totalgrade <=18 && Totalgrade >12)
    {
        grablordbet(2);
    }
    else if(Totalgrade <=12 && Totalgrade >7)
    {
        grablordbet(1);
    }
    else
        grablordbet(0);

}
void robot::RobotThinkPlayhand()
{
    Cards  playerCards = this->GetCards();
    Strategy st(this, playerCards);
    Cards* res = st.GetrootPlayhand();
    this->PlayHand(res);   // 把所有权交给PlayHand，内部会delete
}

robot::robot() {

}
