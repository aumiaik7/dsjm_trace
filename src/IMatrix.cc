#include "IMatrix.hh"
#include <iostream>
#include <cmath>
#include "IRowColumnDS.hh"

using namespace std;

/**
 * @Description : This functions
 * @Creation Date: 20.06.2009
 */
IMatrix::IMatrix(int M, int N, int nz, bool value)
    : IRowColumnDS(M,N,nz, value),
      shouldVerify(false)
{

}
/* IMatrix() ENDS*/

IMatrix::~IMatrix()
{

}

void IMatrix::buildPriorityQueue(int n, int *ndeg, int *head, int *next, int *previous)
{
    int numdeg;
    for (int jp = 1; jp <= n ; jp++)
    {
        numdeg = ndeg[jp];

        previous[jp] = 0;

        next[jp] = head[numdeg];

        if ( head[numdeg] > 0 )
        {
            previous[head[numdeg]] = jp;
        }
        head[numdeg] = jp;
    }
}

/**
 * Constant Operation
 */
void IMatrix::deleteColumn(int *head,int *next,int *previous,int numdeg,int jcol)
{
    if ( previous[jcol] == 0)
    {
        head[numdeg] = next[jcol];
    }
    else
    {
        next[previous[jcol]] = next[jcol];
    }

    if(next[jcol] > 0)
    {
        previous[next[jcol]] = previous[jcol];
    }
}

/**
 * Constant Operation
 */
void IMatrix::addColumn(int *head,int *next,int *previous, int numdeg, int jcol)
{

    previous[jcol] = 0;
    next[jcol] = head[numdeg];
    if(head[numdeg] > 0)
    {
        previous[head[numdeg]] = jcol;
    }
    head[numdeg] = jcol;
}

void IMatrix::initializeDegreesToUVertices(int n,int *tag,int *u_head,int *u_next,int *u_previous, int *u_list, bool *inU, int *u_tag)
{

    for (int jp = 1;jp <= n; jp++)
    {
        u_list[jp] = 0;
        u_head[jp-1] = 0;
        u_next[jp] = 0;
        u_previous[jp] = 0;
        inU[jp] = false;
        u_tag[jp] = 0;
    }
    for (int jp = 1; jp <= n ; jp++)
    {
        if(tag[jp] < n)
        {
            u_previous[jp] = 0;
            u_next[jp] = u_head[0] ;
            if ( u_head[0] > 0)
            {
                u_previous[u_head[0]] = jp;
            }
            u_head[0] = jp;
        }
    }
}

void IMatrix::setVerify(bool v)
{
    shouldVerify = v;
}

bool IMatrix::getVerify()
{
    return shouldVerify;
}

