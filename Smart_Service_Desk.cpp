#include <iostream>
#include <string>
#include <limits>
#include <algorithm>
using namespace std;

struct Customer {
    string id, name, service;
    int arrival = 0, process = 0, weight = 0;
};
struct Ticket { Customer c; bool done = false; };

struct QNode {
    Ticket data; QNode *next = nullptr;
    QNode(const Ticket& t): data(t) {}
};
struct Queue {
    QNode *front = nullptr, *rear = nullptr; int n = 0;
    bool empty() const { return !front; }
    QNode* find(const string& id) {
        for(auto p=front;p;p=p->next) if(p->data.c.id==id) return p;
        return nullptr;
    }
    void push(const Ticket& t) {
        auto p=new QNode(t); if(!rear) front=rear=p; else rear->next=p,rear=p; ++n;
    }
    bool pop(Ticket& t) {
        if(!front) return false; auto p=front; t=p->data; front=p->next;
        if(!front) rear=nullptr; delete p; --n; return true;
    }
    bool remove(const string& id) {
        QNode *p=front,*pre=nullptr;
        while(p) {
            if(p->data.c.id==id) {
                if(pre) pre->next=p->next; else front=p->next;
                if(p==rear) rear=pre; delete p; --n; return true;
            }
            pre=p; p=p->next;
        }
        return false;
    }
    void show() const {
        cout<<"\n--- HANG DOI ---\n";
        if(!front) { cout<<"Rong.\n"; return; }
        int i=1; for(auto p=front;p;p=p->next)
            cout<<i++<<". "<<p->data.c.id<<" | "<<p->data.c.name
                <<" | "<<p->data.c.service<<"\n";
    }
    ~Queue(){ Ticket t; while(pop(t)); }
};

struct LNode {
    Ticket data; LNode *next=nullptr;
    LNode(const Ticket& t): data(t) {}
};
struct List {
    LNode *head=nullptr,*tail=nullptr; int n=0;
    LNode* find(const string& id) {
        for(auto p=head;p;p=p->next) if(p->data.c.id==id) return p;
        return nullptr;
    }
    void add(const Ticket& t) {
        auto p=new LNode(t); if(!tail) head=tail=p; else tail->next=p,tail=p; ++n;
    }
    bool remove(const string& id) {
        LNode *p=head,*pre=nullptr;
        while(p) {
            if(p->data.c.id==id) {
                if(pre) pre->next=p->next; else head=p->next;
                if(p==tail) tail=pre; delete p; --n; return true;
            }
            pre=p; p=p->next;
        }
        return false;
    }
    void show() const {
        cout<<"\n--- DANH SACH ---\n";
        if(!head){cout<<"Rong.\n";return;}
        for(auto p=head;p;p=p->next) {
            auto &c=p->data.c;
            cout<<c.id<<" | "<<c.name<<" | "<<c.service
                <<" | "<<(p->data.done?"Da phuc vu":"Dang cho")<<'\n';
        }
    }
    ~List(){while(head){auto p=head;head=head->next;delete p;}}
};

enum Type { ADD, DEL, UPDATE };
struct Action {
    Type type; Customer oldC,newC; bool waiting=false;
};
struct SNode {
    Action data; SNode *next=nullptr;
    SNode(const Action& a): data(a) {}
};
struct Stack {
    SNode* top=nullptr;
    void push(const Action& a){auto p=new SNode(a);p->next=top;top=p;}
    bool pop(Action& a){if(!top)return false;auto p=top;a=p->data;top=p->next;delete p;return true;}
    ~Stack(){Action a;while(pop(a));}
};

struct Desk {
    Queue q; List list; Stack history;
    int clock=0, served=0, waitTotal=0; long long costTotal=0;

    bool add(const Customer& c,bool save=true){
        if(list.find(c.id)){cout<<"ID da ton tai!\n";return false;}
        Ticket t{c,false}; list.add(t); q.push(t);
        if(save) history.push({ADD,{},c,true});
        cout<<"Da them khach hang.\n"; return true;
    }
    bool del(const string& id,bool save=true){
        auto p=list.find(id); if(!p){cout<<"Khong tim thay!\n";return false;}
        Customer old=p->data.c; bool waiting=!p->data.done && q.find(id);
        q.remove(id); list.remove(id);
        if(save) history.push({DEL,old,{},waiting});
        cout<<"Da xoa.\n"; return true;
    }
    bool update(const string& id,Customer c,bool save=true){
        auto p=list.find(id); if(!p){cout<<"Khong tim thay!\n";return false;}
        Customer old=p->data.c; bool waiting=!p->data.done && q.find(id);
        p->data.c=c; if(auto x=q.find(id)) x->data.c=c;
        if(save) history.push({UPDATE,old,c,waiting});
        cout<<"Da cap nhat.\n"; return true;
    }
    void serve(){
        Ticket t; if(!q.pop(t)){cout<<"Hang doi rong.\n";return;}
        int start=max(clock,t.c.arrival), wait=start-t.c.arrival;
        clock=start+t.c.process; ++served; waitTotal+=wait;
        costTotal+=1LL*wait*t.c.weight;
        if(auto p=list.find(t.c.id)) p->data.done=true;
        cout<<"Da phuc vu "<<t.c.name<<" | Cho "<<wait<<" phut.\n";
    }
    void find(const string& id){
        auto p=list.find(id); if(!p){cout<<"Khong tim thay!\n";return;}
        auto &c=p->data.c;
        cout<<c.id<<" | "<<c.name<<" | "<<c.service<<" | Den:"<<c.arrival
            <<" | Xu ly:"<<c.process<<" | Trong so:"<<c.weight
            <<" | "<<(p->data.done?"Da phuc vu":"Dang cho")<<'\n';
    }
    void stats() const {
        cout<<"\n--- THONG KE ---\nSo khach: "<<list.n
            <<"\nDang cho: "<<q.n<<"\nDa phuc vu: "<<served;
        if(served) cout<<"\nCho TB: "<<(double)waitTotal/served
                       <<" phut\nChi phi cho: "<<costTotal;
        cout<<'\n';
    }
    void undo(){
        Action a; if(!history.pop(a)){cout<<"Khong co thao tac de Undo.\n";return;}
        if(a.type==ADD){q.remove(a.newC.id);list.remove(a.newC.id);}
        else if(a.type==DEL){add(a.oldC,false);}
        else {update(a.oldC.id,a.oldC,false);}
        cout<<"Da Undo.\n";
    }
};

void clear(){cin.clear();cin.ignore(numeric_limits<streamsize>::max(),'\n');}
string str(const string& s){string x;do{cout<<s;getline(cin,x);}while(x.empty());return x;}
int num(const string& s,int min=0){int x;while(cout<<s,!(cin>>x)||x<min){cout<<"Nhap lai!\n";clear();}clear();return x;}
Customer input(){
    Customer c;
    c.id=str("Ma: "); c.name=str("Ten: "); c.service=str("Dich vu: ");
    c.arrival=num("Thoi gian den: "); c.process=num("Thoi gian xu ly: ",1);
    c.weight=num("Trong so: "); return c;
}
void menu(){
    cout<<"\n===== SMART SERVICE DESK =====\n"
        <<"1. Them khach\n2. Xem hang doi\n3. Phuc vu\n4. Xem danh sach\n"
        <<"5. Tim khach\n6. Cap nhat\n7. Xoa\n8. Undo\n9. Thong ke\n0. Thoat\n";
}

int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr); Desk d; int ch;
    do{
        menu(); ch=num("Chon: ");
        switch(ch){
            case 1:d.add(input());break;
            case 2:d.q.show();break;
            case 3:d.serve();break;
            case 4:d.list.show();break;
            case 5:d.find(str("Ma can tim: "));break;
            case 6:{string id=str("Ma can sua: ");auto p=d.list.find(id);
                if(!p){cout<<"Khong tim thay!\n";break;} Customer c=input();c.id=id;d.update(id,c);break;}
            case 7:d.del(str("Ma can xoa: "));break;
            case 8:d.undo();break;
            case 9:d.stats();break;
            case 0:cout<<"Ket thuc.\n";break;
            default:cout<<"Lua chon khong hop le!\n";
        }
    }while(ch!=0);
}
