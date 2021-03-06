#ifndef IMATRIX_HH
#define IMATRIX_HH

#include "IRowColumnDS.hh"
#include "BucketPQ.hh"




class IMatrix : public IRowColumnDS
{
public:
    IMatrix(int M,int N, int nz, bool value);
    virtual ~IMatrix();


    /**
     * Method slo()
     */
    virtual bool slo(int *order) = 0;

    /**
     * Method ido()
     **/
    virtual bool ido(int *order) = 0;
    virtual bool lfo(int *order) = 0 ;

    virtual bool computedegree() = 0 ;
    virtual int greedycolor(int *list, int *ngrp) = 0 ;
    virtual int rlf(int *ngrp) = 0;
    virtual int slo_rlf(int *list, int *ngrp) = 0 ;	  

    void setVerify(bool v);
    bool getVerify();



    void buildPriorityQueue(int n, int *ndeg, int *head, int *next, int *previous);

    void deleteColumn(int *head,int *next,int *previous,int numdeg,int jcol);
    void addColumn(int *head,int *next,int *previous,int numdeg,int jcol);

    void initializeDegreesToUVertices(int n,int *tag,int *u_head,int *u_next,int *u_previous,
                                      int *u_list, bool *f_added, int *f_tag);




    // ========================================
    // RowColumnDS
protected:


    bool shouldVerify;
    int maxdeg;
};

#endif
