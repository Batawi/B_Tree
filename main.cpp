#include <iostream>
#include <cmath> // nan value, isnan() 
#include <algorithm> // std::fill_n
#include <fstream>
#include <string.h>
#include <sstream>
#include <vector>
#include <stdlib.h> // srand, rand
#include <time.h> // time
using namespace std;

#define MIN_NODE_SIZE 1 //d value
#define MAX_NODE_SIZE MIN_NODE_SIZE*2 //2*d
#define RECORD_SIZE 5 //number of numbers in single record
#define INFO_FILE "files/info_file" //name of file with nodes
#define RECORD_FILE "files/record_file" //name of file with records
#define NUMBER_CAP 99

int _no_records;
int _no_nodes;

int _record_reads = 0;
int _record_writes = 0;
int _node_reads = 0;
int _node_writes = 0;

class FindNodeWithKey_retVal
{
public:
    bool found;
    int position;
    int node_pos_in_file;
    int addr_to_parent;
    int position_in_parent; //kid number
};

class FindLeaf_retVal
{
public:
    bool found;
    int node_pos_in_file;
};

class Node
{
public:
    double size;
    double keys[MAX_NODE_SIZE];
    double addr_to_records[MAX_NODE_SIZE];
    double addr_to_kids[MAX_NODE_SIZE+1];

    Node();
    void addNodeToFile();
    bool loadNodeFromFile(int address);
    void printPureNode();
    int nodeSize(); //returning node size in bytes
    FindNodeWithKey_retVal findNodeWithKey(int key, int key_to_look_for);
    void updateThatNode(int address);
    void findParentNode(Node child_node);
    bool operator== (const Node& node) const;
    FindLeaf_retVal findMaxInLeftSubTree(FindNodeWithKey_retVal a);
    FindLeaf_retVal findMinInRigthSubTree(FindNodeWithKey_retVal a);
    bool isItALeaf();
};

void writePureNodes(); //for debug
void writePureRecords(); //for debug
void printPureNodes(); //for debug
void printPureRecords();
bool printPureRecord(int address);
void updateRecord(double record[], int address);
void appendRecord(double record[]);

void fileClear(string name);
void printTree(int depth, int address);
int binarySearch(double arr[], int r, int x);

void countNodesRecords();

bool addKeyRecord(double key, double record[]);
void addKeyRecordMain(Node node, FindNodeWithKey_retVal a, double key, double left_kid, double right_kid);
bool compensation(Node main_node, FindNodeWithKey_retVal a, double key, double left_kid, double right_kid);
void split(Node main_node, FindNodeWithKey_retVal a, double key, double left_kid, double right_kid);
void removeKeyRecord(double key);
void removeKeyRecordMain(Node node, FindNodeWithKey_retVal a, double key);
bool compensationInRemove(Node node, FindNodeWithKey_retVal a, double key);
void menu();
void showAll();
void showAllMain(int address);
void printDiskOp();
void experiment();
// ----- CLASS METHODS -----
Node::Node()
{
    size = 0;
    fill_n(keys, MAX_NODE_SIZE, nan(""));
    fill_n(addr_to_records, MAX_NODE_SIZE, nan(""));
    fill_n(addr_to_kids, MAX_NODE_SIZE+1, nan(""));
}

void Node::addNodeToFile()
{
    ofstream file(INFO_FILE, ios::app | ios::out | ios::binary);
    file.write((char*)&size, sizeof(double));
    file.write((char*)&keys[0], sizeof(double)*MAX_NODE_SIZE);
    file.write((char*)&addr_to_records[0], sizeof(double)*MAX_NODE_SIZE);
    file.write((char*)&addr_to_kids[0], sizeof(double)*(MAX_NODE_SIZE+1));
    _node_writes++;
    file.close();
}

bool Node::loadNodeFromFile(int address) //tu moznaby uzyc seekp()
{
    int node_size = nodeSize(); 
    int j = 1;
    double input_array[node_size/sizeof(double)];

    ifstream file(INFO_FILE, ios::in | ios::binary);
    for(int i=0; i<=address; i++)
    {
        fill_n(input_array, node_size/sizeof(double), nan(""));
        file.read((char*)&input_array[0], node_size);
        _node_reads++;

        if(file.eof())
        {
            return 0;
        }
    }
    
    size = input_array[0];
    for(unsigned int i=0; i<MAX_NODE_SIZE; i++)
    {
        keys[i] = input_array[j];
        j++;
    }

    for(unsigned int i=0; i<MAX_NODE_SIZE; i++)
    {
        addr_to_records[i] = input_array[j];
        j++;
    }

    for(unsigned int i=0; i<MAX_NODE_SIZE+1; i++)
    {
        addr_to_kids[i] = input_array[j];
        j++;
    }

    file.close();
    return 1;
}

int Node::nodeSize()
{
    return sizeof(size)+sizeof(keys)+sizeof(addr_to_records)+sizeof(addr_to_kids);
}

void Node::printPureNode()
{       
    cout<<"Size: "<<size<<endl;
    cout<<"Keys: ";
    for(int i=0; i<MAX_NODE_SIZE; i++)
    {
        cout<<keys[i]<<" ";
    }
    cout<<endl<<"Addresses: ";
    for(int i=0; i<MAX_NODE_SIZE; i++)
    {
        cout<<addr_to_records[i]<<" ";
    }
    cout<<endl<<"Kids: ";
    for(int i=0; i<MAX_NODE_SIZE+1; i++)
    {
        cout<<addr_to_kids[i]<<" ";
    }
    cout<<endl;
}

FindNodeWithKey_retVal Node::findNodeWithKey(int key, int key_to_look_for)
{
    FindNodeWithKey_retVal ret_val;
    int address = 0;
    int parent_address = -1;
    int node_position_in_parent = 0;
    int a;
    while(loadNodeFromFile(address))
    {
        a = binarySearch(keys, size-1, key);
        ret_val.node_pos_in_file = address;
        //found
        if(a != -1)
        {
            ret_val.found = 1;
            ret_val.position = a;
            ret_val.position_in_parent = node_position_in_parent;
            ret_val.addr_to_parent = parent_address;
            return ret_val;
        }

        //not found
        ret_val.found = 0;
        if(key_to_look_for == keys[0])
        {
            ret_val.position_in_parent = node_position_in_parent;
            ret_val.addr_to_parent = parent_address;
            for(int i=0; i<size; i++)
            {
                if(key < keys[i])
                {
                    ret_val.position = i;
                    return ret_val;
                }
            }

            ret_val.position = size;
            return ret_val;
        }
        else if(key < keys[0])
        {
            if(!isnan(addr_to_kids[0]))
            {
                parent_address = address;
                node_position_in_parent = 0;
                address = addr_to_kids[0]; 
            }
            else
            {
                ret_val.position = 0;
                ret_val.position_in_parent = node_position_in_parent;
                ret_val.addr_to_parent = parent_address;
                return ret_val;
            }
        }
        else if(key > keys[int(size-1)])
        {
            if(!isnan(addr_to_kids[int(size)]))
            {
                parent_address = address;
                node_position_in_parent = size;
                address = addr_to_kids[int(size)];
            }
            else
            {
                ret_val.position = size;
                ret_val.position_in_parent = node_position_in_parent;
                ret_val.addr_to_parent = parent_address;
                return ret_val;
            }
        }
        else
        {
            for(int i=1; i<size; i++)
            {
                if(key < keys[i])
                {
                    if(!isnan(addr_to_kids[i]))
                    {
                        parent_address = address;
                        node_position_in_parent = i;
                        address = addr_to_kids[i];
                        break;
                    }
                    else
                    {
                        ret_val.position = i;
                        ret_val.position_in_parent = node_position_in_parent;
                        ret_val.addr_to_parent = parent_address;
                        return ret_val;
                    }
                }
            }
        }
    }

    ret_val.found = 0;
    ret_val.position = 0;
    ret_val.node_pos_in_file = 0;
    return ret_val;
}

void Node::updateThatNode(int address)
{
    ofstream file(INFO_FILE, ios::out | ios::in | ios::binary);
    int node_size = nodeSize(); 
    double output_array[node_size/sizeof(double)];
    int j = 1;

    output_array[0] = size;
    for(unsigned int i=0; i<MAX_NODE_SIZE; i++)
    {
        output_array[j] = keys[i];
        j++;
    }

    for(unsigned int i=0; i<MAX_NODE_SIZE; i++)
    {
        output_array[j] = addr_to_records[i];
        j++;
    }

    for(unsigned int i=0; i<MAX_NODE_SIZE+1; i++)
    {
        output_array[j] = addr_to_kids[i];
        j++;
    }

    //file.open("test", ios::out | ios::in | ios::binary);
    file.seekp(address*node_size, ios::beg);
    file.write((char*)&output_array[0], node_size);
    _node_writes++;
    file.close();
}

void Node::findParentNode(Node child_node)
{
    Node tmp;
    FindNodeWithKey_retVal a = tmp.findNodeWithKey(int(child_node.keys[0]), -1); 
    loadNodeFromFile(a.addr_to_parent);
}

bool Node::operator== (const Node& node) const
{
    if(this->size != node.size)
    {
        return 0;
    }

    for(int i=0; i<this->size; i++)
    {
        if(this->keys[i] != node.keys[i])
        {
            if(isnan(this->keys[i]) == 1 && isnan(node.keys[i]) == 1){}
            else
            {
                return 0;
            }
        }

        if(this->addr_to_records[i] != node.addr_to_records[i])
        {
            if(isnan(this->addr_to_records[i]) == 1 && isnan(node.addr_to_records[i]) == 1){}
            else
            {
                return 0;
            }
        }

        if(this->addr_to_kids[i] != node.addr_to_kids[i])
        {
            if(isnan(this->addr_to_kids[i]) == 1 && isnan(node.addr_to_kids[i]) == 1){}
            else
            {
                cout<<"no siema"<<endl;
                return 0;
            }
        }
    }

    if(this->addr_to_kids[int(this->size)] != node.addr_to_kids[int(node.size)])
    {
        if(isnan(this->addr_to_kids[int(this->size)]) == 1 && isnan(node.addr_to_kids[int(node.size)]) == 1){}
        else
        {
            return 0;
        }
    }

    return 1;
}

FindLeaf_retVal Node::findMaxInLeftSubTree(FindNodeWithKey_retVal a)
{
    FindLeaf_retVal ret;
    ret.node_pos_in_file = a.node_pos_in_file;
    loadNodeFromFile(a.node_pos_in_file); 
    if(isnan(addr_to_kids[a.position]))
    {
        ret.found = 0;
        return ret;
    }
    
    ret.node_pos_in_file = addr_to_kids[a.position];
    loadNodeFromFile(addr_to_kids[a.position]);
    bool end = 0;

    while(end == 0)
    {
        if(isnan(addr_to_kids[int(size)]))
        {
            end = 1;
        }
        else
        {
            ret.node_pos_in_file = int(addr_to_kids[int(size)]);
            loadNodeFromFile(int(addr_to_kids[int(size)])); 
        }

    }
    ret.found = 1;
    return ret;
}

FindLeaf_retVal Node::findMinInRigthSubTree(FindNodeWithKey_retVal a)
{
    FindLeaf_retVal ret;
    ret.node_pos_in_file = a.node_pos_in_file;
    loadNodeFromFile(a.node_pos_in_file); 
    if(isnan(addr_to_kids[a.position+1]))
    {
        ret.found = 0;
        return ret;
    }
    
    ret.node_pos_in_file = addr_to_kids[a.position+1];
    loadNodeFromFile(addr_to_kids[a.position+1]);
    bool end = 0;

    while(end == 0)
    {
        if(isnan(addr_to_kids[0]))
        {
            end = 1;
        }
        else
        {
            ret.node_pos_in_file = int(addr_to_kids[0]);
            loadNodeFromFile(int(addr_to_kids[0])); 
        }

    }
    ret.found = 1;
    return ret;
}

bool Node::isItALeaf()
{
    for(int i=0; i<size+1; i++)
    {
        if(!isnan(addr_to_kids[i]))
        {
            return 0;
        }
    }
    return 1;
}

// ----- FUNCTIONS -----
void writePureNodes() //tutaj mozna bybylo zamienic wszystko na getliny 
{
    fileClear(INFO_FILE);
    int no_nodes;
    int counter;
    Node node;
    string null_check;

    cout<<"How many nodes will You enter? ";
    cin>>no_nodes;
    _no_nodes = no_nodes;

    for(int i=0; i<no_nodes; i++)
    {
        node = Node();
        cout<<"Size: ";
        cin>>node.size;

        cout<<"Key(s): ";
        cin>>ws; //eat up white spaces
        counter = 0;
        while(cin.peek() != '\n')
        {
            cin>>node.keys[counter];
            counter++;
            if(counter == MAX_NODE_SIZE)
            {
                break;
            }
        }

        cout<<"Address(es): ";
        cin>>ws;
        counter = 0;
        while(cin.peek() != '\n')
        {
            cin>>node.addr_to_records[counter];
            counter++;
            if(counter == MAX_NODE_SIZE)
            {
                break;
            }
        }

        cout<<"Kid(s): ";
        cin>>ws; 
        string line="";
        getline(cin, line);
        if(line != "-")
        {
            stringstream ssin(line);
            counter=0;
            while(ssin.good() && counter<MAX_NODE_SIZE+1)
            {
                ssin >> node.addr_to_kids[counter];
                counter++;
            }
        }

        cout<<endl;
        node.addNodeToFile();
    }
}

void writePureRecords()
{
    fileClear(RECORD_FILE);
    ofstream file(RECORD_FILE, ios::app | ios::out | ios::binary);
    
    int no_records;
    int counter;
    string line;
    double record[RECORD_SIZE];
    cout<<"How many records You will write? ";
    cin>>no_records;
    _no_records = no_records; 

    for(int i=0; i<no_records; i++)
    {
        fill_n(record, RECORD_SIZE, nan(""));
        cout<<i+1<<". ";
        cin>>ws; //eat up white spaces
        counter = 0;
        while(cin.peek() != '\n')
        {
            cin>>record[counter];
            counter++;
        }
        for(int i=0; i<RECORD_SIZE; i++)
        {
            cout<<" "<<record[i];
        }
        cout<<endl;

        file.write((char*)&record[0], RECORD_SIZE*sizeof(double));
        _record_writes++;
    }
    file.close();
}

void printPureNodes()
{
    Node node;
    int i = 0;
    while(node.loadNodeFromFile(i))
    {
        node.printPureNode();
        cout<<endl;
        //node = Node();
        i++;
    }
}

void printPureRecords()
{
    ifstream file(RECORD_FILE, ios::in | ios::binary);
    bool finish = 0;
    double record[RECORD_SIZE] = {nan("")};
    while(finish == 0)
    {
        fill_n(record, RECORD_SIZE, nan(""));
        file.read((char*)&record[0], RECORD_SIZE*sizeof(double));
        _record_reads++;
        if(file.eof())
        {
            finish = 1;
        }
        else
        {
            for(int i=0; i<RECORD_SIZE; i++)
            {
                cout<<record[i]<<" ";
            }
            cout<<endl;
        }
    }
    file.close();
}

bool printPureRecord(int address)
{
    ifstream file(RECORD_FILE, ios::in | ios::binary);
    double record[RECORD_SIZE];

    for(int i=0; i<=address; i++)
    {
        fill_n(record, RECORD_SIZE, nan(""));
        file.read((char*)&record[0], RECORD_SIZE*sizeof(double));
        _record_reads++;

        if(file.eof())
        {
            return 0;
        }
    }

    for(int i=0; i<RECORD_SIZE; i++)
    {
        cout<<record[i]<<" ";
    }
    cout<<endl;

    file.close();
    return 1;
}

void updateRecord(double record[], int address)
{
    ofstream file(RECORD_FILE, ios::out | ios::in | ios::binary);

    file.seekp(address*RECORD_SIZE*sizeof(double), ios::beg);
    file.write((char*)&record[0], RECORD_SIZE*sizeof(double));
    _record_writes++;
    file.close();
}

void fileClear(string name)
{
    ofstream file(name, ios::trunc | ios::out | ios::binary);
    file.close();
}

void printTree(int depth, int address)
{
    Node node;
    node = Node();
    if(node.loadNodeFromFile(address) == 0)
    {
        return;
    }
    
    for(int i=0; i<depth; i++)
    {
        cout<<"| ";
    }

    for(int i=0; i<node.size; i++)
    {
        cout<<node.keys[i]<<" ";
    }
    cout<<endl;
    
    for(int i=0; i<MAX_NODE_SIZE+1; i++)
    {
        if(!isnan(node.addr_to_kids[i]))
        {
            printTree(depth+1, node.addr_to_kids[i]);
        }
    }
}

int binarySearch(double arr[], int r, int x)
{
    int l = 0;
    int m = 0;
    while(l <= r)
    {
        //cin.get();
        //m = l + (r-1)/2;
        m = (int)(((l + r) / 2) + 0.5);
        //cout<<l<<" "<<m<<" "<<r<<endl;
        //cout<<" - - rzeczy - -"<<endl;
        if(arr[m] == x)
        {
            return m;
        }

        if(arr[m] < x)
        {
            l = m + 1;
        }
        else
        {
            r = m - 1;
        }
        //cout<<l<<" "<<m<<" "<<r<<endl;
    }
    //cout<<"za whilem"<<endl;
    return -1;
}

void countNodesRecords()
{
    Node node;
    double input_array[node.nodeSize()/sizeof(double)];
    double input_array2[RECORD_SIZE];
    ifstream file;
    bool end = 0;
    _no_records = 0;
    _no_nodes = 0;

    file.open(INFO_FILE, ios::in | ios::binary);
    while(!end)
    {
        file.read((char*)&input_array[0], node.nodeSize());
        _node_reads++;

        if(file.eof())
        {
            end = 1;
        }
        else
        {
            _no_nodes++;
        }
    }
    file.close();

    end = 0;
    file.open(RECORD_FILE, ios::in | ios::binary);
    while(!end)
    {
        file.read((char*)&input_array2[0], RECORD_SIZE*sizeof(double));
        _record_reads++;

        if(file.eof())
        {
            end = 1;
        }
        else
        {
            _no_records++;
        }
    }
    file.close();
}

void appendRecord(double record[])
{
    ofstream file(RECORD_FILE, ios::app | ios::out | ios::binary);
    file.write((char*)&record[0], RECORD_SIZE*sizeof(double));
    _record_writes++;
    file.close();
}

bool addKeyRecord(double key, double record[])
{
    FindNodeWithKey_retVal a;
    Node node;

    a = node.findNodeWithKey(key, -1);
    if(a.found == 1)
    {
        cout<<endl<<"This key already exists!"<<endl;
        return 0;
    }
    else
    {
        addKeyRecordMain(node, a, key, nan(""), nan(""));
        appendRecord(record);
        _no_records++;
        //cout<<" -- CHECK POINT --";
        return 1;
    }

}

void addKeyRecordMain(Node node, FindNodeWithKey_retVal a, double key, double left_kid, double right_kid)
{
    if(_no_nodes == 0)
    {
        _no_nodes++;
    }
    if(node.size < MAX_NODE_SIZE)
    {
        //cout<<"\nWchodze pierwszego nodea\n";
        for(int i=node.size-1; i>=a.position; i--)
        {
            node.keys[i+1] = node.keys[i];
            node.addr_to_records[i+1] = node.addr_to_records[i];
            node.addr_to_kids[i+2] = node.addr_to_kids[i+1];
        }
        node.size++;
        node.keys[a.position] = key;
        node.addr_to_records[a.position] = _no_records;
        node.addr_to_kids[a.position+1] = right_kid;
        node.updateThatNode(a.node_pos_in_file);
    }
    else if(compensation(node, a, key, left_kid, right_kid)){}
    else
    {
        //cout<<"\nWchodze do splita\n";
        split(node, a, key, left_kid, right_kid);
    }
}

bool compensation(Node main_node, FindNodeWithKey_retVal a, double key, double left_kid, double right_kid)
{
    Node parent_node;
    Node sibling_node;

    bool load_ret_val;
    bool left_sibling; //is sibling to the left of main node
    
    int addr_to_sibling_node;
    vector <double> keys_to_distribute;
    vector <double> records_to_distribute;
    vector <double> kids_to_distribute;

    parent_node.loadNodeFromFile(a.addr_to_parent);

    //checking for available sibling node
    if(a.position_in_parent == 0)
    {
        load_ret_val = sibling_node.loadNodeFromFile(parent_node.addr_to_kids[1]);
        left_sibling = 0;
    }
    else if(a.position_in_parent == parent_node.size)
    {
        load_ret_val = sibling_node.loadNodeFromFile(parent_node.addr_to_kids[int(parent_node.size-1)]);
        left_sibling = 1;
    }
    else
    {
        load_ret_val = sibling_node.loadNodeFromFile(parent_node.addr_to_kids[a.position_in_parent-1]);
        left_sibling = 1;
        if(load_ret_val == 0 || sibling_node.size == MAX_NODE_SIZE)
        {
            load_ret_val = sibling_node.loadNodeFromFile(parent_node.addr_to_kids[a.position_in_parent+1]);
            left_sibling = 0;
            if(load_ret_val == 0 || sibling_node.size == MAX_NODE_SIZE)
            {
                return 0;
            }
        }
    }
    
    if(load_ret_val == 0 || sibling_node.size == MAX_NODE_SIZE)
    {
        return 0; //not success
    }

    if(left_sibling == 0) //setting addr to sibling node
    {
        addr_to_sibling_node = parent_node.addr_to_kids[a.position_in_parent+1];
    }
    else
    {
        addr_to_sibling_node = parent_node.addr_to_kids[a.position_in_parent-1];
    }

    // ---- MAIN PART ----
    if(left_sibling == 0)
    {
        //cout<<"wchodze do left = 0"<<endl;
        //stworzenie tablic z wartosciami do dystrybucji
        keys_to_distribute.clear();
        records_to_distribute.clear();
        kids_to_distribute.clear();
        for(int i=0; i<main_node.size; i++)
        {
            keys_to_distribute.push_back(main_node.keys[i]);
            records_to_distribute.push_back(main_node.addr_to_records[i]);
            kids_to_distribute.push_back(main_node.addr_to_kids[i]);
        }
        kids_to_distribute.push_back(main_node.addr_to_kids[int(main_node.size)]);

        keys_to_distribute.insert(keys_to_distribute.begin()+a.position, key);
        records_to_distribute.insert(records_to_distribute.begin()+a.position, _no_records);

        keys_to_distribute.push_back(parent_node.keys[a.position_in_parent]);
        records_to_distribute.push_back(parent_node.addr_to_records[a.position_in_parent]);
        
        for(int i=0; i<sibling_node.size; i++)
        {
            keys_to_distribute.push_back(sibling_node.keys[i]);
            records_to_distribute.push_back(sibling_node.addr_to_records[i]);
            kids_to_distribute.push_back(sibling_node.addr_to_kids[i]);
        }
        kids_to_distribute.push_back(sibling_node.addr_to_kids[int(sibling_node.size)]);
        kids_to_distribute.insert(kids_to_distribute.begin()+a.position+2, right_kid);
        /*for(unsigned int j=0; j<kids_to_distribute.size(); j++)
        {
            cout<<kids_to_distribute[j]<<"   ";
        }*/


        //przypisanie odpowiednich wartosci do node'ow
        int mid_point = keys_to_distribute.size()/2;
        parent_node.keys[a.position_in_parent] = keys_to_distribute[mid_point];
        parent_node.addr_to_records[a.position_in_parent] = records_to_distribute[mid_point];

        fill_n(main_node.keys, MAX_NODE_SIZE, nan(""));
        fill_n(main_node.addr_to_records, MAX_NODE_SIZE, nan(""));
        fill_n(main_node.addr_to_kids, MAX_NODE_SIZE+1, nan(""));
        main_node.size = 0;
        for(int i=0; i<mid_point; i++)
        {
            main_node.size++;
            main_node.keys[i] = keys_to_distribute[i];
            main_node.addr_to_records[i] = records_to_distribute[i];
            main_node.addr_to_kids[i] = kids_to_distribute[i];
        }
        main_node.addr_to_kids[mid_point] = kids_to_distribute[mid_point];

        fill_n(sibling_node.keys, MAX_NODE_SIZE, nan(""));
        fill_n(sibling_node.addr_to_records, MAX_NODE_SIZE, nan(""));
        fill_n(sibling_node.addr_to_kids, MAX_NODE_SIZE+1, nan(""));
        sibling_node.size = 0;
        for(unsigned int i=mid_point+1; i<keys_to_distribute.size(); i++)
        {
            sibling_node.size++;
            sibling_node.keys[i-(mid_point+1)] = keys_to_distribute[i];
            sibling_node.addr_to_records[i-(mid_point+1)] = records_to_distribute[i];
            sibling_node.addr_to_kids[i-(mid_point+1)] = kids_to_distribute[i];
        }
        //sibling_node.size++;
        sibling_node.addr_to_kids[int(sibling_node.size)] = kids_to_distribute[keys_to_distribute.size()];

        
        /*cout<<endl;
        parent_node.printPureNode();
        cout<<endl;
        main_node.printPureNode();
        cout<<endl;
        sibling_node.printPureNode();
        cout<<endl;*/
        

        //update node'ow
        main_node.updateThatNode(a.node_pos_in_file);
        parent_node.updateThatNode(a.addr_to_parent);
        sibling_node.updateThatNode(addr_to_sibling_node);
    }
    else
    {
        //cout<<"wchodze do left = 1"<<endl;
        //stworzenie tablic z wartosciami do dystrybucji
        //cout<<endl<<" --- I CYK ---"<<endl;
        keys_to_distribute.clear();
        records_to_distribute.clear();
        kids_to_distribute.clear();
        for(int i=0; i<sibling_node.size; i++)
        {
            keys_to_distribute.push_back(sibling_node.keys[i]);
            records_to_distribute.push_back(sibling_node.addr_to_records[i]);
            kids_to_distribute.push_back(sibling_node.addr_to_kids[i]);
        }
        kids_to_distribute.push_back(sibling_node.addr_to_kids[int(sibling_node.size)]);

        keys_to_distribute.push_back(parent_node.keys[a.position_in_parent-1]);
        records_to_distribute.push_back(parent_node.addr_to_records[a.position_in_parent-1]);
        
        for(int i=0; i<main_node.size; i++)
        {
            keys_to_distribute.push_back(main_node.keys[i]);
            records_to_distribute.push_back(main_node.addr_to_records[i]);
            kids_to_distribute.push_back(main_node.addr_to_kids[i]);
        }
        kids_to_distribute.push_back(main_node.addr_to_kids[int(main_node.size)]);
        kids_to_distribute.insert(kids_to_distribute.begin()+a.position+1, right_kid);

        keys_to_distribute.insert(keys_to_distribute.begin()+sibling_node.size+a.position+1, key);
        records_to_distribute.insert(records_to_distribute.begin()+sibling_node.size+a.position+1, _no_records);
        /*for(unsigned int j=0; j<kids_to_distribute.size(); j++)
        {
            cout<<kids_to_distribute[j]<<"   ";
        }*/

        //przypisanie odpowiednich wartosci do node'ow
        int mid_point = keys_to_distribute.size()/2;
        parent_node.keys[a.position_in_parent-1] = keys_to_distribute[mid_point];
        parent_node.addr_to_records[a.position_in_parent-1] = records_to_distribute[mid_point];

        fill_n(sibling_node.keys, MAX_NODE_SIZE, nan(""));
        fill_n(sibling_node.addr_to_records, MAX_NODE_SIZE, nan(""));
        fill_n(sibling_node.addr_to_kids, MAX_NODE_SIZE+1, nan(""));
        sibling_node.size = 0;
        for(int i=0; i<mid_point; i++)
        {
            sibling_node.size++;
            sibling_node.keys[i] = keys_to_distribute[i];
            sibling_node.addr_to_records[i] = records_to_distribute[i];
            sibling_node.addr_to_kids[i] = kids_to_distribute[i];
        }
        sibling_node.addr_to_kids[mid_point] = kids_to_distribute[mid_point];
        //sibling_node.size++;

        fill_n(main_node.keys, MAX_NODE_SIZE, nan(""));
        fill_n(main_node.addr_to_records, MAX_NODE_SIZE, nan(""));
        fill_n(main_node.addr_to_kids, MAX_NODE_SIZE+1, nan(""));
        main_node.size = 0;
        for(unsigned int i=mid_point+1; i<keys_to_distribute.size(); i++)
        {
            main_node.size++;
            main_node.keys[i-(mid_point+1)] = keys_to_distribute[i];
            main_node.addr_to_records[i-(mid_point+1)] = records_to_distribute[i];
            main_node.addr_to_kids[i-(mid_point+1)] = kids_to_distribute[i];
        }
        main_node.addr_to_kids[int(main_node.size)] = kids_to_distribute[keys_to_distribute.size()];

        /*cout<<endl;
        parent_node.printPureNode();
        cout<<endl;
        sibling_node.printPureNode();
        cout<<endl;
        main_node.printPureNode();
        cout<<endl;*/

        //update node'ow
        main_node.updateThatNode(a.node_pos_in_file);
        parent_node.updateThatNode(a.addr_to_parent);
        sibling_node.updateThatNode(addr_to_sibling_node);
    }

    return 1; //success
}

void split(Node main_node, FindNodeWithKey_retVal a, double key, double left_kid, double right_kid)
{
    Node parent_node;
    Node new_node;

    vector <double> keys_to_distribute;
    vector <double> records_to_distribute;
    vector <double> addr_to_distribute;

    parent_node.loadNodeFromFile(a.addr_to_parent);

    //przepisanie wartosci do vectorow
    for(int i=0; i<main_node.size; i++)
    {
        keys_to_distribute.push_back(main_node.keys[i]);
        records_to_distribute.push_back(main_node.addr_to_records[i]);
        addr_to_distribute.push_back(main_node.addr_to_kids[i]);
    }
    addr_to_distribute.push_back(main_node.addr_to_kids[int(main_node.size)]);

    keys_to_distribute.insert(keys_to_distribute.begin()+a.position, key);
    records_to_distribute.insert(records_to_distribute.begin()+a.position, _no_records);

    //addr_to_distribute.insert(addr_to_distribute.begin()+addr_to_distribute.size()/2+1, right_kid);
    addr_to_distribute.insert(addr_to_distribute.begin()+a.position+1, right_kid);

    /*for(unsigned int i=0; i<addr_to_distribute.size(); i++)
    {
        cout<<addr_to_distribute[i]<<endl;
    }*/

    
    //przypisanie wartosci do node'ow
    int mid_point = keys_to_distribute.size()/2;
    fill_n(main_node.keys, MAX_NODE_SIZE, nan(""));
    fill_n(main_node.addr_to_records, MAX_NODE_SIZE, nan(""));
    fill_n(main_node.addr_to_kids, MAX_NODE_SIZE+1, nan(""));
    main_node.size = mid_point;
    for(int i=0; i<mid_point; i++)
    {
        main_node.keys[i] = keys_to_distribute[i];
        main_node.addr_to_records[i] = records_to_distribute[i];
        main_node.addr_to_kids[i] = addr_to_distribute[i];
    }
    main_node.addr_to_kids[mid_point] = addr_to_distribute[mid_point];

    fill_n(new_node.keys, MAX_NODE_SIZE, nan(""));
    fill_n(new_node.addr_to_records, MAX_NODE_SIZE, nan(""));
    fill_n(new_node.addr_to_kids, MAX_NODE_SIZE+1, nan(""));
    new_node.size = keys_to_distribute.size()-(mid_point+1); // <--- ??
    for(unsigned int i=mid_point+1; i<keys_to_distribute.size(); i++)
    {
        new_node.keys[i-(mid_point+1)] = keys_to_distribute[i];
        new_node.addr_to_records[i-(mid_point+1)] = records_to_distribute[i];
        new_node.addr_to_kids[i-(mid_point+1)] = addr_to_distribute[i];
    }
    new_node.addr_to_kids[int(new_node.size)] = addr_to_distribute[keys_to_distribute.size()];

    /*cout<<endl;
    main_node.printPureNode();
    cout<<endl;
    new_node.printPureNode();
    cout<<endl;*/
    //cout<<endl<<"---- "<<keys_to_distribute[mid_point]<<endl;
    //return;
    
    //gdzies tutaj musze dodac warunek na sprawdzenie czy pojawi sie nowy root


    key = keys_to_distribute[mid_point];
    if(a.addr_to_parent == -1)
    {
        //cout<<"jestesmy juz w rootcie"<<endl;
        main_node.addNodeToFile();
        new_node.addNodeToFile();

        // --- tworzenie nowego roota ---
        new_node = Node();
        new_node.size = 1;
        new_node.keys[0] = key;
        new_node.addr_to_records[0] = records_to_distribute[mid_point];
        new_node.addr_to_kids[0] = _no_nodes;
        _no_nodes++; 
        new_node.addr_to_kids[1] = _no_nodes;
        _no_nodes++; 
        new_node.updateThatNode(0);
    }
    else
    {
        main_node.updateThatNode(a.node_pos_in_file);
        new_node.addNodeToFile();
        _no_nodes++;
        a = parent_node.findNodeWithKey(key, parent_node.keys[0]);
        addKeyRecordMain(parent_node, a, key, left_kid, _no_nodes-1);
    }
}

void removeKeyRecord(double key)
{
    FindNodeWithKey_retVal a;
    Node node;

    a = node.findNodeWithKey(key, -1);
    if(a.found == 0)
    {
        cout<<"There is no such key!";
    }
    else
    {
        removeKeyRecordMain(node, a, key);
    }
}

void removeKeyRecordMain(Node node, FindNodeWithKey_retVal a, double key)
{
    Node leaf_node;
    FindLeaf_retVal ret;
    bool removed = 0;
    if(node.isItALeaf())
    {
        if(node.size > 1)
        {
            for(int i=0; i<node.size-1-a.position; i++)
            {
                node.keys[i+a.position] = node.keys[i+a.position+1];
                node.addr_to_records[i+a.position] = node.addr_to_records[i+a.position+1];
            }
            node.size--;
            node.keys[int(node.size)] = nan("");
            node.addr_to_records[int(node.size)] = nan("");
            node.updateThatNode(a.node_pos_in_file);
            removed = 1;
        }
        else if(compensationInRemove(node, a, key))
        {
            removed = 1;
        }
    }

    if(removed == 0) 
    {
        ret = leaf_node.findMaxInLeftSubTree(a);
        if(ret.found == 1 && leaf_node.size > 1)
        {
            //cout<<endl<<" --- RYM CYM CYM ---"<<endl;
            //cout<<leaf_node.keys[int(leaf_node.size)-1]<<endl;
            node.keys[a.position] = leaf_node.keys[int(leaf_node.size)-1];
            node.addr_to_records[a.position] = leaf_node.addr_to_records[int(leaf_node.size)-1];

            leaf_node.size--;
            leaf_node.keys[int(leaf_node.size)] = nan("");
            leaf_node.addr_to_records[int(leaf_node.size)] = nan("");
            leaf_node.updateThatNode(ret.node_pos_in_file);
            node.updateThatNode(a.node_pos_in_file);
            removed = 1;
        }
        else
        {
            ret = leaf_node.findMinInRigthSubTree(a);
            if(ret.found == 1 && leaf_node.size > 1)
            {
                node.keys[a.position] = leaf_node.keys[0];
                node.addr_to_records[a.position] = leaf_node.addr_to_records[0];

                for(int i=0; i<leaf_node.size-1; i++)
                {
                    leaf_node.keys[i] = leaf_node.keys[i+1];
                    leaf_node.addr_to_records[i] = leaf_node.addr_to_records[i+1];
                }

                leaf_node.size--;
                leaf_node.keys[int(leaf_node.size)] = nan("");
                leaf_node.addr_to_records[int(leaf_node.size)] = nan("");
                leaf_node.updateThatNode(ret.node_pos_in_file);
                node.updateThatNode(a.node_pos_in_file);
                removed = 1;
            }
            else if(compensationInRemove(node, a, key))
            {
                removed = 1;
            }
        }
    }

    if(removed == 0)
    {
        //merge;
        cout<<"I can't remove this key"<<endl;
    }
}

bool compensationInRemove(Node node, FindNodeWithKey_retVal a, double key)
{
    return 0;
}

void showAll()
{
    Node node;
    node.loadNodeFromFile(0);
    showAllMain(0);
}

void showAllMain(int address)
{
    Node node;
    node.loadNodeFromFile(address);
    if(node.isItALeaf())
    {
        for(int i=0; i<node.size; i++)
        {
            cout<<"Key: "<<node.keys[i]<<endl;
            cout<<"Record: ";
            printPureRecord(node.addr_to_records[i]);
            cout<<endl;
        }
    }
    else
    {
        for(int i=0; i<node.size+1; i++)
        {
            showAllMain(node.addr_to_kids[i]);

            if(!isnan(node.keys[i]))
            {
                cout<<"Key: "<<node.keys[i]<<endl;
                cout<<"Record: ";
                printPureRecord(node.addr_to_records[i]);
                cout<<endl;
            }
        }
    }
}

void printDiskOp()
{
    cout<<"Records reads: "<<_record_reads<<". Records writes: "<<_record_writes<<"."<<endl;
    cout<<"Nodes   reads: "<<_node_reads<<". Nodes   writes: "<<_node_writes<<"."<<endl;

    _record_reads = 0;
    _record_writes = 0;
    _node_reads = 0;
    _node_writes = 0;
}

void experiment()
{
    fileClear(INFO_FILE);
    fileClear(RECORD_FILE);
    _no_nodes = 0;
    _no_records = 0;
    _record_reads = 0;
    _record_writes = 0;
    _node_reads = 0;
    _node_writes = 0;
    //ofstream file("files/exp1", ios::trunc | ios::out);
    //ofstream file("files/exp2", ios::trunc | ios::out);
    //ofstream file("files/exp3", ios::trunc | ios::out);
    ofstream file("files/exp4", ios::trunc | ios::out);

    int no_records_to_create = 8000;
    int counter = 0;
    double key;
    double record[RECORD_SIZE];
    double x; //zajetosc procentowa
    
    //stworzenie przykladowego rekordu
    fill_n(record, RECORD_SIZE, nan(""));
    for(int i=0; i<RECORD_SIZE/2; i++)
    {
        record[i] = i+1;
    }

    while(counter < no_records_to_create)
    {
        //key = counter + 1;
        key = rand()%50000+1;

        if(addKeyRecord(key, record))
        {
            cout<<"fasdfasd"<<endl;
            x = (counter+1)*100;
            x /= _no_nodes;
            x /= MAX_NODE_SIZE;
            file<<x;
            file<<endl;

            /*file<<_node_writes<<" "<<_node_reads<<endl;

            _record_reads = 0;
            _record_writes = 0;
            _node_reads = 0;
            _node_writes = 0;*/

            counter++;
        }
    }

    file.close();
}

void menu()
{
    _record_writes = 0;
    _node_writes = 0;

    Node node;
    int counter;
    int no_numbers;
    char input;
    double record[RECORD_SIZE];
    double key;
    FindNodeWithKey_retVal a;
    bool exit = 0;
    while(exit == 0)
    {
        cout<<"Option: ";
        cin>>input;
        cout<<endl;

        switch(input)
        {
            case 'h':
                cout<<"------------- HELP SCREEN -------------"<<endl
                    <<" Print tree                        - t"<<endl
                    <<" Pring info file                   - i"<<endl
                    <<" Print records file                - r"<<endl
                    <<" Add key and record                - *"<<endl
                    <<" Add key with random record        - +"<<endl
                    <<" Print key with it's record        - k"<<endl
                    <<" Update record                     - u"<<endl
                    <<" Print number of records and nodes - n"<<endl
                    <<" Delete key                        - d"<<endl
                    <<" Show all                          - s"<<endl
                    <<" Print pure nodes                  - o"<<endl
                    <<" Print pure records                - c"<<endl
                    <<" Write pure node                   - w"<<endl
                    <<" Write pure records                - p"<<endl
                    <<" Clear both files                  - f"<<endl
                    <<" Print this help                   - h"<<endl
                    <<" Exit                              - e"<<endl;
                break;

            case 'f':
                cout<<"Both files cleared"<<endl;
                fileClear(INFO_FILE);
                fileClear(RECORD_FILE);
                _no_nodes = 0;
                _no_records = 0;
                _record_writes = 1;
                _node_writes = 1;
                break;
            case 'o':
                printPureNodes();
                break;
            case 'c':
                printPureRecords();
                break;
            case 'w':
                writePureNodes();
                break;
            case 'p':
                writePureRecords();
                break;

            case 't':
                printTree(0, 0);
                break;

            case 'i':
                printPureNodes();
                break;

            case 'r':
                printPureRecords();
                break;

            case '*':
                fill_n(record, RECORD_SIZE, nan(""));
                counter = 0;
                cout<<"Write key: ";
                cin>>key;
                cout<<"Write record: ";
                cin>>ws;
                while(cin.peek() != '\n')
                {
                    cin>>record[counter];
                    counter++;
                }
                addKeyRecord(key, record);
                break;

            case '+':
                fill_n(record, RECORD_SIZE, nan(""));
                counter = 0;
                cout<<"Write key: ";
                cin>>key;

                no_numbers = rand() % RECORD_SIZE + 1;

                cout<<"Generated record: ";
                for(int i=0; i<no_numbers; i++)
                {
                    record[i] = rand() % NUMBER_CAP;
                    cout<<record[i]<<" ";
                }
                cout<<endl;
                addKeyRecord(key, record);
                break;

            case 'k':
                cout<<"What key you are looking for? ";
                cin>>key;
                a = node.findNodeWithKey(key, -1); 
                if(a.found == 0)
                {
                    cout<<"There is no such key!"<<endl;
                }
                else
                {
                    printPureRecord(node.addr_to_records[a.position]);
                }
                break;

            case 'u':
                cout<<"Record of what key You want to update? ";
                cin>>key;
                a = node.findNodeWithKey(key, -1); 
                if(a.found == 0)
                {
                    cout<<"There is no such key!"<<endl;
                }
                else
                {
                    fill_n(record, RECORD_SIZE, nan(""));
                    counter = 0;
                    cout<<"This is current record: "<<endl;
                    printPureRecord(node.addr_to_records[a.position]);
                    cout<<"Write new record:\n";
                    cin>>ws;
                    while(cin.peek() != '\n')
                    {
                        cin>>record[counter];
                        counter++;
                    }

                    updateRecord(record, node.addr_to_records[a.position]);
                }
                break;

            case 'n':
                cout<<endl<<"Number of nodes: "<<_no_nodes<<endl;
                cout<<"Number of records: "<<_no_records<<endl;
                break;

            case 'd':
                cout<<"Which key You want remove? ";
                cin>>key;
                removeKeyRecord(key);
                break;

            case 's': //show all
                showAll();
                break;

            case 'e':
                exit = 1;
                break;

            default:
                cout<<"Wrong command!"<<endl;
                break;
        }
        cout<<endl;
        printDiskOp();
        cout<<" -------------------------------------- "<<endl;
    }
}

int main()
{
    srand(time(NULL));
    //experiment();
    
    countNodesRecords();
    menu();
    //cout<<endl<<_no_nodes<<endl;

    /*FindNodeWithKey_retVal a;
    FindLeaf_retVal ret;
    a = node.findNodeWithKey(7, -1);
    ret = node.findMaxInLeftSubTree(a);
    node.printPureNode();
    cout<<endl<<ret.node_pos_in_file<<endl;*/
    //printPureNodes();


    //writePureNodes();
    //writePureRecords();
    
    //fileClear(RECORD_FILE);
    //printPureRecords();
    //printPureRecord(4);
    //node.updateThatNode(2);
    
    //printPureNodes();
    //double arr[] = {6, 6, 6, 7, 7};
    //updateRecord(arr, 0);
    //printPureRecords();
    //printTree(0, 0);
    //cout<<endl;
    //addKeyRecord(6, arr);
    //cout<<endl;
    //printPureNodes();
    //printTree(0, 0);

    return 0;
}
