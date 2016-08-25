////========================================================
// FileName:         PVR-Tree
// Created on:      2016/8/25
// Author:              Chen-Ping Pei  
////========================================================

/*Copyright (c) 2016. Chen-Ping Pei   (chen_ping.pei@foxmail.com)*/

#include<stdio.h>
#include<stdlib.h>
#include <math.h>

#include "RTree.h"
#include "boost/polygon/voronoi.hpp"
#include "boost/polygon/voronoi_diagram.hpp"

using namespace boost::polygon;

struct Rect
{
  Rect()  {}

  Rect(int a_minX, int a_minY, int a_maxX, int a_maxY)
  {
    min[0] = a_minX;
    min[1] = a_minY;

    max[0] = a_maxX;
    max[1] = a_maxY;
  }
  int min[2];
  int max[2];
};


double* max(double a[20][4],int num);
double* min(double a[20][4],int num);
bool MySearchCallback(int id, void* arg) ;

typedef int coordinate_type;
typedef point_data<coordinate_type> Point;
typedef voronoi_diagram<double> VD;
typedef voronoi_edge<double> VE;

int FileNum=5922;//数据点行数（自行输入）
int pNum=-1;//返回数据点编号
std::vector<Point> points;//数据点集
double d=10000000;//初始距离
double Qx,Qy;//查询点

int main(int argc, char* argv[])
{
    //步骤1.将数据点从文件中取出并生成维诺图。
    VD vd;//保存维诺图的变量
    RTree<int, int, 2, float> tree;//R树结构
    FILE *fp;
    char buf[300];//读取文件的缓冲区
    double x,y;//坐标点数据
    fp=fopen("B.txt","r");
    double xxx,yyy;
    for(int i=0;i<FileNum;i++)
    {
        fgets(buf,300,fp);
        sscanf(buf,"%lf %lf",&x,&y);   
        points.push_back(Point(x*1000000,y*1000000));//将数据点插入生成维诺图的点数据结构
    }
    fclose(fp);
    construct_voronoi(points.begin(), points.end(), &vd);//生成维诺图

    //步骤2.将维诺图构成R树
    int num=0;//单元格个数
    VE::voronoi_edge_type* ie;
    for (VD::const_cell_iterator it = vd.cells().begin(); it != vd.cells().end(); ++it)
    {
        int flag=1;//当flag=0时不做处理continue
        int meNum=0;//边(点)数量
        double me[20][4]={0};//当前单元内包含的边的端点坐标
        ie=(VE::voronoi_edge_type*)(it->incident_edge());//取当前单元的一条边
        if(ie->vertex0()==NULL||ie->vertex1()==NULL)//当边为无无限长时不做处理
        {
            flag=0;
            continue;
        }
        x=ie->vertex0()->x();//首边坐标
        y=ie->vertex0()->y();
        while(1)
        {
            ie=(VE::voronoi_edge_type*)(ie->next()); 
            if(ie->vertex0()==NULL||ie->vertex1()==NULL)//当边为无无限长时不做处理
            {
                flag=0;
                break;
            }
            else
            {
                if(ie->vertex0()->x()==x&&ie->vertex0()->y()==y&&meNum!=0)//该单元所有遍遍历完毕
                    break;
                me[meNum][0]=ie->vertex0()->x();
                me[meNum][1]=ie->vertex0()->y();
                me[meNum][2]=ie->vertex1()->x();
                me[meNum][3]=ie->vertex1()->y();
                meNum=meNum+1;
            }
        }
        if(flag==0)
            continue;
        //循环至此已将该单元所有边遍历完毕并存储。
        double *M1;
        double *M2;
        int rMax[2];
        int rMin[2];
        M1=max(me,meNum);//取多边形外接矩形坐标。
        rMax[0]=ceil(M1[0]);rMax[1]=ceil(M1[1]);
        M2=min(me,meNum);
        rMin[0]=ceil(M2[0]);rMin[1]=ceil(M2[1]);
        struct Rect rect(rMin[0],rMin[1],rMax[0],rMax[1]);//初始化叶子节点（区域）
        tree.Insert(rect.min, rect.max, num);//将叶子节点插入r树
        num++;
    }
    //以上过程为构造r树、以下过程为查询。
    printf("\n请输入查询点以空格分割：");
    scanf("%lf%lf",&Qx,&Qy);
    Rect search_rect(Qx,Qx,Qy,Qy);
    int nhits=tree.Search(search_rect.min, search_rect.max, MySearchCallback, NULL);
    if(pNum==-1)
    {
        printf("\n查无数据！\n");
        return 0;
    }
    Point point;
    point=points[pNum];
    printf("\n最近点编号：%d     坐标为（%d,%d）\n",pNum,point.x(),point.y());
    getchar();
    return 0;
}

double* max(double a[20][4],int num)
{
    double nowMax[2];
    nowMax[0]=a[0][0];
    nowMax[1]=a[0][1];
    for(int i=0;i<num;i++)
    {
        if(a[i][0]>nowMax[0])
            nowMax[0]=a[i][0];
        if(a[i][1]>nowMax[1])
            nowMax[1]=a[i][1];
    }
    return nowMax;
} 

double* min(double a[20][4],int num)
{
    double nowMin[2];
    nowMin[0]=a[0][0];
    nowMin[1]=a[0][1];
    for(int i=0;i<num;i++)
    {
        if(a[i][0]<nowMin[0])
            nowMin[0]=a[i][0];
        if(a[i][1]<nowMin[1])
            nowMin[1]=a[i][1];
    }
    return nowMin;
}

bool MySearchCallback(int id, void* arg) 
{
    Point point;
    point=points[id];	
    double temp=((Qx-point.x())*(Qx-point.x())+(Qy-point.y())*(Qy-point.y()))/1000000000;
    int aaa=point.x();
    int bbb=point.y();
    if(temp<d)
    {
        d=temp;
        pNum=id;
    }
    return true; // keep going
}

