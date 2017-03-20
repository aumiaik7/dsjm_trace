// -*- mode: c++; fill-column:80 ; comment-fill-column: 80; -*-
// (setq fill-column 100)
#include <iostream>
#include <cmath>
#include <cassert>
#include <stdexcept>

#include "Matrix.hh"

#include "BucketPQ.hh"
#include "Utility.h"
#include "RLF.hh"

using namespace std;


/**
 * Constructor
 */
Matrix::Matrix(int M,int N,int nz, bool value)
    : IMatrix(M,N,nz, value)
{
    ndeg = new int[N+1];
}

/**
 * Destructor
 */
Matrix::~Matrix()
{

  delete[] ndeg;
}


/**
 * Purpose: 		Computes Degree sequence of the columns of a sparse matrix A (i.e. the vertices
 *          		of the column intersection graph G(A) ).
 *
 * Pre-condition: 	The matrix object is nonempty. Assumes that the
 *                  computeCCS(), compress() and computeCRS() has been called prior calling this
 *                  function, such that matrix object holds the sparsity
 *                  information in Compressed Column and Compressed Row
 *                  storage.
 *
 * Post-condition: 	Degree information for the columns of matrix A ( graph G(A) ) is stored  in
 * 			        the data member <id:ndeg>, an integer array
 * 			        of size n+1 such that if k =  ndeg[j] then  the column j has
 * 			        degree k, where j = 1,2, ...,n.
 *
 * Return values:   Returns true when the function is executed successfully,
 *                  otherwise returns false.
 *
 */
bool Matrix::computedegree()
{
    maxdeg = -1;
    int *tag;
    try
    {
        tag = new int[N+1]; // Temporary working array of size N+1. If w[jcol] =
                          // N, then the degree of column jcol has been
                          // computed.


        // Initialize <id:ndeg> and <id:w>
        for(int jp = 1; jp <= N; jp++)
        {
            ndeg[jp] = 0;
            tag[jp] = 0;
        }

        // At each step, choose a column <id:jcol> and visit all
        // the adjacent columns to compute degree in the intersection graph
        // G(A).
        for(int jcol = 2; jcol <= N; jcol++)
        {
            tag[jcol] = N;
            for(int jp = jpntr[jcol]; jp <= jpntr[jcol+1]-1 ;jp++)
            {
                int ir = row_ind[jp];
                for (int ip = ipntr[ir]; ip <=  ipntr[ir+1]-1 ;ip++  )
                {
                    int ic = col_ind[ip];
                    if (tag[ic] < jcol)
                    {
                        tag[ic] = jcol;
                        ndeg[ic] = ndeg[ic] + 1;
                        ndeg[jcol] = ndeg[jcol] + 1;
                        maxdeg = max(ndeg[jcol],maxdeg);
                        maxdeg = max(ndeg[ic], maxdeg);
                    }
                }
            }
        }
    }
    catch(bad_alloc)
    {
        delete[] tag;
        return false;
    }
    if(maxdeg == -1)
        maxdeg = 0;
    delete[] tag;
    return true;
}


/**
 * Purpose:             Computes the greedy coloring of the columns of a sparse
 *                      matrix A (i.e. the vertices of the column intersection
 *                      graph G(A))
 *
 * Pre-condition:       The matrix object is nonempty. Assumes that an ordering
 *                      has been provided in the in-parameter <id:order> integer
 *                      array of size n+1, such that order[1]...order[n] is a
 *                      permutation of {1,...,n}
 *
 * Post-condition:      The greedy coloring of Matrix A( graph G(A) ) is stored
 *                      in the in-out-parameter <id:color>, an integer array of
 *                      size n+1, such that if k = color[j] then the column j is
 *                      colored with color k, j = 1,2,...,n
 *
 *
 * Parameters:          in-parameter <id:order>, an integer pointer to an
 *                      array of size n+1, containing a permutation of
 *                      {1,...,n}. The integer array uses 1-based indexing.
 *
 *                      in-out-parameter <id:color>, an integer pointer to an
 *                      array of size n+1, it stores the color values of the
 *                      columns in successful completion.The integer array uses
 *                      1-based indexing.
 *
 * Return values:       Returns the number of colors if succeeds, otherwise
 *                      returns 0(zero).
 */


int Matrix::greedycolor(int *order, int *color)
{
    if(order == NULL || color == NULL)
        return 0;

    int *tag;                     // working array of size n+1. It is used to mark
                                // the colors already used for adjacent columns
    int maxgrp = 0;
    int ic,ip,ir,j,jcol,jp;
    try
    {
        tag = new int[N+1];       // working array of size n+1

        for (jp = 1; jp <=  N ;jp++  ) // Initialization of the arrays.
        {
            color[jp] = N;
            tag[jp] = 0;
        }

        for (int seq = 1; seq <=  N ;seq++  ) // Colors are assigned to each column taken
                                          // from the <id:order> array
                                          // sequentially.
        {
            jcol = order[seq]; // Pick a column, according to the ordering.

            // Find all columns adjacent to column jcol.
            for (jp = jpntr[jcol]; jp <  jpntr[jcol+1] ;jp++  )
            {
                ir = row_ind[jp];
                for (ip = ipntr[ir]; ip <  ipntr[ir+1] ;ip++  )
                {
                    ic = col_ind[ip];
                    // Mark the color number with seq number
                    tag[color[ic]] = seq;
                }
            }

            // Assign the smallest un-marked color number to jcol.
            for (jp = 1; jp <=  maxgrp ;jp++  )
            {
                if (tag[jp] != seq)
                {
                	break;
                	//goto SEQ_L50;
                }
            }
            if(jp>maxgrp)
            	maxgrp = maxgrp + 1;
        //SEQ_L50:
            color[jcol] = jp;
        }
        delete[] tag;
        numberOfColors = maxgrp;
        return maxgrp;
    }
    catch(bad_alloc)
    {
        delete[] tag;
        return 0;
    }
}


/**
 * Purpose: 		Computes Smallest-Last Ordering (SLO) of the columns of a sparse matrix A (i.e. the vertices
 *          		of the column intersection graph G(A) )
 *
 * Pre-condition: 	The matrix object is nonempty. Assumes that the degree of
 *                  of the columns have already been computed in the data member
 *                  <id:ndeg> integer array of size n+1 using computeDegree() method.
 *
 * Post-condition: 	The SLO ordering of matrix A ( graph G(A) ) is stored  in
 * 			        the out-parameter <id:list>, an integer array
 * 			        of size n+1 such that if k =  list[j] then  the column j is the k-th element,
 *			        k = 1,2, ..., n, in the SLO ordering, and j = 1,2, ...,
 * 			        n.
 *
 * Parameters:      out-parameter <id:list>, an integer pointer to an array of size n+1. The array will
 *                  contain the ordering information when the function normally
 *                  returns.
 *
 * Return values:   Returns true when the function is executed successfully,
 *                  otherwise returns false.
 *
 */

bool Matrix::slo(int *list)
{
    int mindeg, numord;
    if(list == NULL)
        return false;


    vector<int> tag;

    try
    {
        tag.reserve(N+1);
        BucketPQ<MinQueue> priority_queue(maxdeg,N);
        mindeg = N;

        for(int jp=1;jp <= N; jp++)
        {
            priority_queue.insert(jp,ndeg[jp]); // assume that ndeg has already been
            // computed (by computeDegree() method)
            tag[jp] = N;
            mindeg = min(mindeg,ndeg[jp]);
        }

        int maximalClique = 0; // Reset maximalClique. It will be set in the while loop
                           // only once.
        numord = N; // numord stores the ordering number for the next column to be
                    // processed. It also indicates the number of columns remaining
                    // to be processed.

        while(1)
        {
            int ic,ip, ir, jcol, jp, numdeg;
            /*
             * We find the largest clique when number of columns remaining is
             * equal to mindeg+1.
             */
            if ((mindeg +1 == numord ) && (maximalClique == 0) )
            {
                maximalClique = numord;
            }

            // find column jcol with minimal degree
            Item item = priority_queue.top();
            jcol = item.index;
            mindeg = item.priority;

            priority_queue.pop();

            list[numord] = jcol;
            numord = numord -1;

            // when numord = 0, we have already processed all the columns
            if (numord == 0)
            {
                return true;
            }

            tag[jcol] = 0;


            // Determine all nonzero entries (ir,jcol)


            for(jp = jpntr[jcol]; jp <= jpntr[jcol+1] -1;jp++)
            {
                ir = row_ind[jp] ;

                // For each row ir,determine all nonzero entries (ir,ic)
                for(ip = ipntr[ir] ; ip <= ipntr[ir+1] - 1; ip++)
                {
                    ic = col_ind[ip];
                    /* Array tag marks columns which are adjacent to
                     * column jcol
                     */

                    if(tag[ic] > numord)
                    {

                        tag[ic] = numord;

                        // Update the degree in the priority queue.
                        priority_queue.decrease(ic);
                        numdeg = priority_queue.get(ic).priority;
                        mindeg = min(mindeg,numdeg);

                    }
                }
            }
        }
    }
    catch(bad_alloc) // for vector.reserve()
    {
        return false;
    }
    catch(length_error) // for vector.reserve()
    {
        return false;
    }

}


/**
 * Purpose: 		Computes Incidence-Degree Ordering (IDO) of the columns of a sparse matrix A (i.e. the vertices
 *          		of the column intersection graph G(A) )
 *
 * Pre-condition: 	The matrix object is nonempty. Assumes that the degree of
 *                  of the columns have already been computed in the data member
 *                  <id:ndeg> integer array of size n+1 using computeDegree() method.
 *
 * Post-condition: 	The IDO ordering of matrix A ( graph G(A) ) is stored  in
 * 			        the out-parameter <id:order>, an integer array
 * 			        of size n+1 such that if k =  order[j] then  the column j is the k-th element,
 *			        k = 1,2, ..., n, in the IDO ordering, and j = 1,2, ...,
 * 			        n.
 *
 * Parameters:      out-parameter <id:order>, an integer pointer to an array of size n+1. The array will
 *                  contain the ordering information when the function normally
 *                  returns.
 *
 * Return values:   Returns true when the function is executed successfully,
 *                  otherwise returns false.
 *
 */
bool Matrix::ido(int *order )
{
    int *head,*previous, *next, *tag, *inducedDeg;
    try
    {

        // The following three integer arrays consist of a doubly linked list. It acts
        // as a bucket priority queue for the incidence degree of the columns.

        // head(deg) is the first column in the deg list unless head(deg) =
        // 0. If head(deg) = 0 there are no columns in the deg list.

        // previous(col) is the column before col in the incidence list unless
        // previous(col) = 0. If previous(col) = 0,  col is the first column in this
        // incidence list.

        // next(col) is the column after col in the incidence list unless
        // next(col) = 0. If next(col) = 0,  col is the last column in this incidence
        // list.

	// inducedDeg(col) is the degree of column in G(V\V')

        // if col is in un-ordered column, then order[col] is the incidence
        // degree of col to the graph induced by the ordered columns. If col is
        // an ordered column, then order[col] is the incidence-degree order of
        // column col.

        head = new int[N];
        previous = new int[N+1];
        next = new int[N+1];
	inducedDeg = new int[N+1]; 

        tag = new int[N+1]; // Temporary array, used for marking ordered columns

        // Sort the indices of degree array <id:ndeg> in descending order, i.e
        // ndeg(tag(i)) is in descending order , i = 1,2,...,n
        //
        // <id:tag> is used here as an in-out-parameter to <id:indexSort> routine. It
        // will hold the sorted indices. The two arrays, <id:previous> and
        // <id:next> is used for temporary storage required for <id:indexSort>
        // routine.
        MatrixUtility::indexsort(N,N-1,ndeg,-1,tag/* index*/ ,previous/* last
                                                                       */ ,next/* next
                                                                                */ );
        // Initialize the doubly linked list, and <id:tag> and <id:order> integer array.
        for(int jp =N ; jp >= 1 ; jp--)
        {
            int ic = tag[jp]; /* Tag is sorted indices for now */
            head[N-jp] = 0;

            addColumn(head,next,previous,0,ic);

            tag[jp] = 0;
            order[jp] = 0;
	    inducedDeg[jp] = ndeg[jp];	
        }

        // determine the maximal search length to search for maximal degree in
        // the maximal incidence degree list.
        int maxLast = 0;
        for(int ir =1 ; ir <= M ; ir++)
        {
            maxLast = maxLast + MatrixUtility::square(ipntr[ir+1] - ipntr[ir]);
        }
        maxLast = maxLast/N;

        int maximalClique = 0;

        int maxinc = 0;
        int ncomp;
        int numord = 1;
        do
        {
            // update the size of the largest clique
            // found during the ordering.
        	if (maxinc == 0)
                ncomp = 0;
            ncomp = ncomp + 1;
            if (maxinc + 1 == ncomp)
                maximalClique = max(maximalClique,ncomp);


            // choose a column jcol of maximal incidence degree
            int jcol;
            {
                int jp;
                do{
                    jp = head[maxinc];
                    if (jp > 0)
                        break;
                    maxinc = maxinc - 1;
                }while(1);


                // We search a distance of maxLast length to find the column with
                // maximal degree in the original graph.
                for(int numlst = 1,  numwgt = -1; numlst <= maxLast; numlst++)
                {
                    //if (ndeg[jp] > numwgt)
		    if (inducedDeg[jp] > numwgt)	
                    {
                        //numwgt = ndeg[jp];
			numwgt = inducedDeg[jp];
                        jcol = jp;
                    }
                    jp = next[jp];
                    if (jp <= 0)
                        break;
                }
            }

            order[jcol] = numord;
            numord = numord + 1;

            // termination test.
            if( numord > N)
                break;

            // delete column jcol from the maxinc order.
            deleteColumn(head,next,previous,maxinc,jcol);


            tag[jcol] = N;

            // Find all columns adjacent to jcol
            for(int jp = jpntr[jcol] ; jp <= jpntr[jcol+1] -1; jp++)
            {
                int ir = row_ind[jp];
                for(int ip = ipntr[ir];ip <=  ipntr[ir+1]-1; ip++)
                {
                    int ic = col_ind[ip];

                    if (tag[ic] < numord)
                    {
                        tag[ic] = numord;

                        // update the pointers to the current incidence lists.
                        int incidence = order[ic];
                        order[ic] = order[ic] + 1;

                        // update the maxinc.
                        maxinc = max(maxinc,order[ic]);

                        // delete column ic from the incidence list.
                        deleteColumn(head,next,previous,incidence,ic);


                        // add column ic to the incidence+1 list.
                        addColumn(head,next,previous,incidence+1,ic);
			inducedDeg[ic] = inducedDeg[ic] - 1; 
                    }
                }
            }
        }while(1);

        // Invert the integer array <id:order>
        for(int jcol = 1;jcol<= N; jcol++)
        {
            previous[order[jcol]] = jcol;
        }
        for(int jp = 1;jp <= N; jp++)
        {
            order[jp] = previous[jp];
        }

    }
    catch (bad_alloc)
    {
        cerr << "Memory Exhausted in Matrix::IDO\n";

        if(head) delete[] head;
        if(previous) delete[] previous;
        if(next) delete[] next;
        if(tag) delete[] tag;

        return false;
    }

    if(head) delete[] head;
    if(previous) delete[] previous;
    if(next) delete[] next;
    if(tag) delete[] tag;
    if(inducedDeg) delete[] inducedDeg;	

    return true;

}



/**
 * Purpose: 		Computes Recursive Largest-First coloring (RLF) of the columns of a sparse matrix A (i.e. the vertices
 *          		of the column intersection graph G(A) )
 *
 * Pre-condition: 	The matrix object is nonempty. Assumes that the degree of
 *                  of the columns have already been computed in the data member
 *                  <id:ndeg> integer array of size n+1 using computeDegree() method.
 *
 * Post-condition: 	RLF coloring of Matrix A(graph G(A)) is stored in the
 * 			        in-out-parameter <id:color>, an integer array of size n+1,
 * 			        such that if k =  color[j] then the column j is colored with
 * 			        color k, j = 1,2,...,n
 *
 * Parameters:      out-parameter <id:color>, an integer pointer to an array of size n+1. The array will
 *                  contain the color values of the columns in successful
 *                  completion. The integer array uses 1-based indexing.
 *
 *
 * Return values:   Returns the number of colors if succeeds, otherwise returns
 *                  0(zero).
 *
 */
int Matrix::rlf(int *color)
{

    /*
     * Overview:
     * In RLF coloring algorithm, we maintain three sets of vertices in three
     * sets,
     *     1. set V for the admissible columns, initially it contains all the
     *        from the graph G(A).
     *     2. set U for the columns non-admissible to current color class q. At
     *        start of a new color class this set is empty.
     *     3. set C for the colored class.
     *
     * At the start of each color class we choose a column jcol with the maximal
     * degree in set V.
     * At other steps we choose a column jcol from set V , which has the maximal
     * number of neighbors in set U, we call it U-Degree.
     *
     * As each column is chosen, it is colored with the value of the current
     * color class q, and moved from the set V to C. All the adjacent columns
     * are moved to set U, as inadmissible columns for the current. set.
     *
     * As columns are added to set U, we update the U-Degree of each column in
     * set V.
     *
     * Coloring is finished when all the columns are colored.
     */

    int *tag;
    int *blackList;

    bool *inU;
    int *u_tag;

    try
    {
        BucketPQ<MaxQueue> u_queue(this->maxdeg, N); // Priority queue for
                                                     // choosing column from set
                                                     // U.
        BucketPQ<MaxQueue> priority_queue(this->maxdeg, N); // Priority queue
                                                            // for choosing
                                                            // column from set
                                                            // V.

        tag = new int[N+1]; // For a column jcol, if tag[jcol] = N, then this
                            // column has already been colored. If 0 < tag[jcol]
                            // (= numord) < N, then jcol has been processed for
                            // a column in numord step.


        blackList = new int[M+1]; // If, blackList[irow] = q, where q is the
                                  // color class and irow is a row number,  then any column
                                  // having nonzero element in irow-th row
                                  // cannot be included in the q-th color
                                  // class. We maintain this array to gain
                                  // better performance in RLF.


        // Initialize BlackList array.
        for ( int i = 1 ; i <= M; i++)
        {
            blackList[i] = 0;
        }


        inU = new bool[N+1]; // If, inU[jcol] = true, then jcol is a member of
                             // set U at that time.

        u_tag = new int[N+1]; // Works similarly as in <id:tag> array, but
                              // applicable to columns of set U only. If
                              // u_tag[ic] = jcol, then the column ic has been
                              // processed for column jcol.




        int u_maxdeg = 0;


        int q = 1; // Current color class, each column picked is colored to the
                   // value of q.

        int maxdeg = 0;

        int numord = 1; // Holds the order value/step of choosing column for
                        // coloring. We increase the value by 1 after coloring
                        // each column.

        int countU = 0; // Number of elements in set U
        int countV = N; // Number of elements in set V
        int countC = 0; // Number of elements in set C
        int count = 0;


        // Initialize the integer arrays <id:tag>, <id:inU> and <id:u_tag>.
        for(int jp = 1; jp <=N ; jp++)
        {
            tag[jp] = 0;
            maxdeg = max(maxdeg,ndeg[jp]);
            inU[jp] = false;

            u_tag[jp] = 0;
        }

        // Initialize both of the prioirty queues.
        for (int jp = 1; jp <= N ; jp++)
        {
            priority_queue.insert(jp, ndeg[jp]);
            u_queue.insert(jp, 0);
        }
        bool newColorClass = true; // Flag variable to indicate whether we
                                   // have just picked a column for a new
                                   // color class or not. It stays true for
                                   // the first column in each color class.

        while(true)
        {
            int jcol;


            if (newColorClass == true)
            {
                newColorClass = false;

                // Choose a column jcol of maximal degree from
                // <id:priority_queue>

                Item item = priority_queue.top();
                jcol = item.index;
                maxdeg = item.priority;

            }
            else
            {

                // Choose a column jcol that has maximal degree in set U.
                Item item = u_queue.top();
                jcol = item.index;
                u_maxdeg = item.priority;
            }

            // Update the number counters.
            countV--;
            countC++;


            // Color the chosen column jcol with the value of current color
            // class.
            color[jcol] = q;
            tag[jcol] = N;

            numord++;

            // Termination Test.
            // If N number of columns has already been colored, then terminate
            // this function an return the total number of colors used.
            if(numord > N)
            {
                // De-allocate Memory.
                if(tag) delete[] tag;

                if(inU) delete[] inU;
                if(u_tag) delete[] u_tag;

                if(blackList) delete[] blackList;
                numberOfColors = q;
                return q;
            }

            // Removed the colored column jcol from both of the priority
            // queues.
            priority_queue.remove(jcol);
            u_queue.remove(jcol);


            // Blacklist all the rows which have nonzero elements in the chosen
            // column jcol.
            // We do not process any of the columns found on this blacklist,
            // while updating the u_degree/priority for each of the vertices.
            for(int jp = jpntr[jcol] ; jp < jpntr[jcol+1] ; jp++)
            {
                int ir = row_ind[jp];
                blackList[ir] = q;
            }


            // Find all adjacent columns of jcol, and move them to set U.
            for ( int jp = jpntr[jcol] ; jp < jpntr[jcol+1]  ; jp++)
            {
                int ir = row_ind[jp];

                for ( int ip = ipntr[ir]; ip < ipntr[ir+1]; ip++)
                {
                    int ic= col_ind[ip];

                    if(tag[ic] < numord) // if this adjacent column is not
                                         // processed for jcol.
                    {
                        tag[ic] = numord;
                        priority_queue.decrease(ic);

                        // Move the column in set U.
                        if (inU[ic] == false)
                        {
                            inU[ic] = true;
                            countU++;
                            countV--;

                            u_queue.remove(ic);

                            // Update the U_degrees of the adjacent vertices.
                            u_maxdeg = RLF::pq_updateDegreesToUVertices(N,ic,u_maxdeg, jpntr,row_ind,ipntr,col_ind,inU,
                                                                        tag,u_tag,u_queue,blackList,q);

                        }
                    }
                }
            }

            // countV + countC + countU == N.
            // If countV = 0, the set of admissible columns  V is empty. We
            // start a new color class, and reset the priority queue for
            // elements in set U.
            if ( countV == 0)
            {

                // Start a new Color Class or Independent set.
                q = q + 1;


                newColorClass = true;

                // Swap values.
                countV =  countU;
                countU = 0;


                u_maxdeg = 0;

                // Reset the priority queues for the elements in set U.
                RLF::pq_initializeDegreesToUVertices(N,tag,u_queue,inU,u_tag);
            }
        }
    }

    catch(bad_alloc)
    {
        cerr << "ERROR: Memory Exhausted" << endl;

        if(tag) delete[] tag;

        if(inU) delete[] inU;
        if(u_tag) delete[] u_tag;
        if(blackList) delete[] blackList;

        return 0;
    }
}

int Matrix::slo_rlf(int *list,int *color)
{

    /*
     * Overview:
     * In RLF coloring algorithm, we maintain three sets of vertices in three
     * sets,
     *     1. set V for the admissible columns, initially it contains all the
     *        from the graph G(A).
     *     2. set U for the columns non-admissible to current color class q. At
     *        start of a new color class this set is empty.
     *     3. set C for the colored class.
     *
     * At the start of each color class we choose a column jcol with the maximal
     * degree in set V.
     * At other steps we choose a column jcol from set V , which has the maximal
     * number of neighbors in set U, we call it U-Degree.
     *
     * As each column is chosen, it is colored with the value of the current
     * color class q, and moved from the set V to C. All the adjacent columns
     * are moved to set U, as inadmissible columns for the current. set.
     *
     * As columns are added to set U, we update the U-Degree of each column in
     * set V.
     *
     * Coloring is finished when all the columns are colored.
     */

    int *tag;
    int *blackList;

    bool *inU;
    int *u_tag;

    try
    {
        BucketPQ<MaxQueue> u_queue(this->maxdeg, N); // Priority queue for
                                                     // choosing column from set
                                                     // U.
        BucketPQ<MaxQueue> priority_queue(this->maxdeg, N); // Priority queue
                                                            // for choosing
                                                            // column from set
                                                            // V.

        tag = new int[N+1]; // For a column jcol, if tag[jcol] = N, then this
                            // column has already been colored. If 0 < tag[jcol]
                            // (= numord) < N, then jcol has been processed for
                            // a column in numord step.


        blackList = new int[M+1]; // If, blackList[irow] = q, where q is the
                                  // color class and irow is a row number,  then any column
                                  // having nonzero element in irow-th row
                                  // cannot be included in the q-th color
                                  // class. We maintain this array to gain
                                  // better performance in RLF.


        // Initialize BlackList array.
        for ( int i = 1 ; i <= M; i++)
        {
            blackList[i] = 0;
        }


        inU = new bool[N+1]; // If, inU[jcol] = true, then jcol is a member of
                             // set U at that time.

        u_tag = new int[N+1]; // Works similarly as in <id:tag> array, but
                              // applicable to columns of set U only. If
                              // u_tag[ic] = jcol, then the column ic has been
                              // processed for column jcol.




        int u_maxdeg = 0;


        int q = 1; // Current color class, each column picked is colored to the
                   // value of q.

        int maxdeg = 0;

        int numord = 1; // Holds the order value/step of choosing column for
                        // coloring. We increase the value by 1 after coloring
                        // each column.

        int countU = 0; // Number of elements in set U
        int countV = N; // Number of elements in set V
        int countC = 0; // Number of elements in set C
        int count = 0;


        // Initialize the integer arrays <id:tag>, <id:inU> and <id:u_tag>.
        for(int jp = 1; jp <=N ; jp++)
        {
            tag[jp] = 0;
            maxdeg = max(maxdeg,ndeg[list[jp]]);
            inU[jp] = false;

            u_tag[jp] = 0;
        }

        // Initialize both of the prioirty queues.
        for (int jp = 1; jp <= N ; jp++)
        {
	    //cout<<list[jp]<<" "<<ndeg[list[jp]]<<";";
            priority_queue.insert(list[jp], ndeg[list[jp]]);
            u_queue.insert(jp, 0);
        }
        bool newColorClass = true; // Flag variable to indicate whether we
                                   // have just picked a column for a new
                                   // color class or not. It stays true for
                                   // the first column in each color class.

        while(true)
        {
            int jcol;


            if (newColorClass == true)
            {
                newColorClass = false;

                // Choose a column jcol of maximal degree from
                // <id:priority_queue>
                Item item = priority_queue.top();
                jcol = item.index;
                maxdeg = item.priority;
		

            }
            else
            {

                // Choose a column jcol that has maximal degree in set U.
                Item item = u_queue.top();
                jcol = item.index;
                u_maxdeg = item.priority;
		
            }

            // Update the number counters.
            countV--;
            countC++;


            // Color the chosen column jcol with the value of current color
            // class.
            color[jcol] = q;
            tag[jcol] = N;

            numord++;

            // Termination Test.
            // If N number of columns has already been colored, then terminate
            // this function an return the total number of colors used.
            if(numord > N)
            {
                // De-allocate Memory.
                if(tag) delete[] tag;

                if(inU) delete[] inU;
                if(u_tag) delete[] u_tag;

                if(blackList) delete[] blackList;
                numberOfColors = q;
                return q;
            }

            // Removed the colored column jcol from both of the priority
            // queues.
            priority_queue.remove(jcol);
            u_queue.remove(jcol);


            // Blacklist all the rows which have nonzero elements in the chosen
            // column jcol.
            // We do not process any of the columns found on this blacklist,
            // while updating the u_degree/priority for each of the vertices.
            for(int jp = jpntr[jcol] ; jp < jpntr[jcol+1] ; jp++)
            {
                int ir = row_ind[jp];
                blackList[ir] = q;
            }


            // Find all adjacent columns of jcol, and move them to set U.
            for ( int jp = jpntr[jcol] ; jp < jpntr[jcol+1]  ; jp++)
            {
                int ir = row_ind[jp];

                for ( int ip = ipntr[ir]; ip < ipntr[ir+1]; ip++)
                {
                    int ic= col_ind[ip];

                    if(tag[ic] < numord) // if this adjacent column is not
                                         // processed for jcol.
                    {
                        tag[ic] = numord;
                        priority_queue.decrease(ic);

                        // Move the column in set U.
                        if (inU[ic] == false)
                        {
                            inU[ic] = true;
                            countU++;
                            countV--;

                            u_queue.remove(ic);

                            // Update the U_degrees of the adjacent vertices.
                            u_maxdeg = RLF::pq_updateDegreesToUVertices(N,ic,u_maxdeg, jpntr,row_ind,ipntr,col_ind,inU,
                                                                        tag,u_tag,u_queue,blackList,q);

                        }
                    }
                }
            }

            // countV + countC + countU == N.
            // If countV = 0, the set of admissible columns  V is empty. We
            // start a new color class, and reset the priority queue for
            // elements in set U.
            if ( countV == 0)
            {

                // Start a new Color Class or Independent set.
                q = q + 1;


                newColorClass = true;

                // Swap values.
                countV =  countU;
                countU = 0;


                u_maxdeg = 0;

                // Reset the priority queues for the elements in set U.
                RLF::pq_initializeDegreesToUVertices(N,tag,u_queue,inU,u_tag);
            }
        }
    }

    catch(bad_alloc)
    {
        cerr << "ERROR: Memory Exhausted" << endl;

        if(tag) delete[] tag;

        if(inU) delete[] inU;
        if(u_tag) delete[] u_tag;
        if(blackList) delete[] blackList;

        return 0;
    }
}


/**
 * Purpose: 		Computes Largest-First Ordering (LFO) of the columns of a sparse matrix A (i.e. the vertices
 *          		of the column intersection graph G(A) )
 *
 * Pre-condition: 	The matrix object is nonempty. Assumes that the degree of
 *                  of the columns have already been computed in the data member
 *                  <id:ndeg> integer array of size n+1 using computeDegree() method.
 *
 * Post-condition: 	The LFO ordering of matrix A ( graph G(A) ) is stored  in
 * 			        the out-parameter <id:order>, an integer array
 * 			        of size n+1 such that if k =  order[j] then  the column j is the k-th element,
 *			        k = 1,2, ..., n, in the LFO ordering, and j = 1,2, ...,
 * 			        n.
 *
 * Parameters:      out-parameter <id:order>, an integer pointer to an array of size n+1. The array will
 *                  contain the ordering information when the function normally
 *                  returns.
 *
 * Return values:   Returns true when the function is executed successfully,
 *                  otherwise returns false.
 *
 */
bool Matrix::lfo(int *order)
{
    int *head;
    int *previous;
    int *next;


    try
    {
        // The following three integer arrays consist of a doubly linked list. It acts
        // as a bucket priority queue for the incidence degree of the columns.

        // head(deg) is the first column in the deg list unless head(deg) =
        // 0. If head(deg) = 0 there are no columns in the deg list.

        // previous(col) is the column before col in the incidence list unless
        // previous(col) = 0. If previous(col) = 0,  col is the first column in this
        // incidence list.

        // next(col) is the column after col in the incidence list unless
        // next(col) = 0. If next(col) = 0,  col is the last column in this incidence
        // list.
        head = new int[N+1];
        previous = new int[N+1];
        next = new int[N+1];


        int maxdeg = -1;

        for(int jp=1;jp <= N; jp++)
        {
            head[jp-1] = 0 ; // We use degree as an index to find a column from
                             // the head list, which ranges from 0,..., n-1.
            maxdeg = max(maxdeg,ndeg[jp]);
        }


        // Initialize the Priority Queue
        buildPriorityQueue(N,ndeg,head,next,previous);

        int numord = 1;
        int jcol;
        while(true)
        {
            // choose a column jcol of maximal degree
            do
            {
                jcol = head[maxdeg];
                if (jcol > 0)
                    break;
                maxdeg = maxdeg -1 ;
            }while(true);

            order[jcol] = numord;
            numord = numord +1;

            // Termination test.
            if (numord > N )
            {
                delete[] head;
                delete[] next;
                delete[] previous;
                return true;
            }


            // Delete Jcol from the head of the list.
            head[maxdeg] = next[jcol];

            if(next[jcol] > 0)
            {
                previous[next[jcol]] = 0;
            }
        }
    }
    catch(bad_alloc)
    {
        delete[] next ;
        delete[] head;
        delete[] previous;

        return false;
    }

}

/* class SLOTempMemory
{
public:
  SLOTempMemory()
    :
  {}
};
*/

 /**
  *
  * @Description:
  *  This method is called when we move a vertex from set V'
  *  to set U.
  * This method has a complexity of \sum{\rho_i}
  */

 /**
  *
  * @Description:
  *  This method is called when we move a vertex from set V'
  *  to set U.
  * This method has a complexity of \sum{\rho_i}
  */



/**
 * Purpose: 		Computes Saturation-Degree Coloring(or Ordering) (SDO) of the columns of a sparse matrix A (i.e. the vertices
 *          		of the column intersection graph G(A) )
 *
 * Pre-condition: 	The matrix object is nonempty. Assumes that the degree of
 *                  of the columns have already been computed in the data member
 *                  <id:ndeg> integer array of size n+1 using computeDegree() method.
 *
 * Post-condition: 	SDO coloring of Matrix A(graph G(A)) is stored in the
 * 			        in-out-parameter <id:color>, an integer array of size n+1,
 * 			        such that if k =  color[j] then the column j is colored with
 * 			        color k, j = 1,2,...,n
 *
 * Parameters:      out-parameter <id:color>, an integer pointer to an array of size n+1. The array will
 *                  contain the color values of the columns in successful
 *                  completion. The integer array uses 1-based indexing.
 *
 *
 * Return values:   Returns the number of colors if succeeds, otherwise returns
 *                  0(zero).
 *
 */
int Matrix::sdo(int *color)
{
    int *satDeg = NULL;
    int *head = NULL;
    int *next = NULL;
    int *previous = NULL;
    int *tag = NULL;
    int *seqTag = NULL;
    int maxgrp = 0;
    int *inducedDeg = NULL;	

    boost::dynamic_bitset<>** bitsets;

   
    


    // for(boost::dynamic_bitset<>::size_type i = 0; i < x.size(); i++)
    //     std::cout << x[i];

    // std::cout << "\n";
    // std::cout << x << "\n"; 


    try
    {
        // The following three integer arrays consist of a doubly linked satDeg. It acts
        // as a bucket priority queue for the incidence degree of the columns.

        // head(deg) is the first column in the deg satDeg unless head(deg) =
        // 0. If head(deg) = 0 there are no columns in the deg satDeg.

        // previous(col) is the column before col in the incidence satDeg unless
        // previous(col) = 0. If previous(col) = 0,  col is the first column in this
        // incidence satDeg.

        // next(col) is the column after col in the incidence satDeg unless
        // next(col) = 0. If next(col) = 0,  col is the last column in this incidence
        // satDeg.

        head     = new int[N];
        next     = new int[N+1];
        previous = new int[N+1];

        tag      = new int[N+1]; // for each unordered column, tag[jcol] stores
                                 // the number of order it has been processed for,
                                 // and for ordered/colored column, it stores N

        seqTag      = new int[N+1]; // This array of size n+1 is used for
                                    // searching the lowest possible color for a
                                    // column jcol.

        satDeg = new int[N+1]; // Array of size n+1, for each unordered column j,
                               // satDeg[j] is the saturation degree, where j =
                               // 1,2,3,...,n.
                               // For each ordered column j, satDeg[j] is the
                               // order in Staruation Degree Ordering.
        inducedDeg = new int[N+1]; //Degree of columns in G(V\V')

        bitsets = new boost::dynamic_bitset<>*[N+1]; 




        // Sort the indices of degree array <id:ndeg> in descending order, i.e
        // ndeg(tag(i)) is in descending order , i = 1,2,...,n
        //
        // <id:tag> is used here as an in-out-parameter to <id:indexSort> routine. It
        // will hold the sorted indices. The two arrays, <id:previous> and
        // <id:next> is used for temporary storage required for <id:indexSort>
        // routine.
        MatrixUtility::indexsort(N,N-1, ndeg,-1,tag , previous, next);

        // Initialize the doubly linked list, <id:satDeg>, and <id:tag> and <id:order> integer array.
        for (int jp = N; jp >= 1; jp--)
        {
            int ic = tag[jp]; /* Tag is sorted indices for now */
            head[N-jp] = 0;

            addColumn(head,next,previous,0,ic);

            tag[jp] = 0;
            satDeg[jp] = 0;
            color[jp] = N;
            seqTag[jp] = 0;
            *(bitsets+jp) = NULL; 
	    inducedDeg[jp] = ndeg[jp];
        }

        int maximalClique = 0;
        int numord = 1;

        // determine the maximal search length to search for maximal degree in
        // the maximal incidence degree satDeg.
        int maxlst = 0;

        for( int ir = 1; ir <= M; ir++)
        {
            maxlst = maxlst + MatrixUtility::square(ipntr[ir+1] - ipntr[ir]);
        }

        maxlst = maxlst / N;

        int maxsat = 0;
        while(true)
        {
            int jp;
            int jcol;
            // Find a column jp with the maximal saturation degree.
            while(true)
            {
                jp = head[maxsat];
                if(jp > 0)
                    break;
                maxsat--;
            }

            // We search a distance of maxLast length to find the colum with
            // maximal degree in the original graph.
            for(int numlst = 1,numwgt = -1;  numlst <= maxlst; numlst++)
            {
                //if(ndeg[jp] > numwgt)
		if(inducedDeg[jp] > numwgt)
                {
                    //numwgt = ndeg[jp];
		    numwgt = inducedDeg[jp];	
                    jcol = jp;
                }
                jp = next[jp];
                if(jp <= 0)
                    break;
            }
	    //cout<<"Column :"<<jcol<<" Degree: "<<ndeg[jcol]<<" Induced Deg: "<<inducedDeg[jcol]<<endl;	
            // To Color the column <id:jcol> with smallest possible number
            // we find all columns adjacent to column <id:jcol>.
            // and find the color that is not used yet.

            for(int jp = jpntr[jcol] ; jp < jpntr[jcol+1]  ; jp++)
            {
                int ir = row_ind[jp];

                for( int ip = ipntr[ir]; ip < ipntr[ir + 1] ; ip++)
                {
                    int ic = col_ind[ip];
                    seqTag[color[ic]] = jcol;
                }
            }

            int newColor;
            for (newColor = 1; newColor <= maxgrp; newColor++)
            {
                if(seqTag[newColor] != jcol)
               		break;
		//     goto SDO_L50;
            }
	    //changed this to remove the goto statement
            if(newColor>maxgrp)	
            {
			maxgrp = maxgrp + 1;

			// This position means we are creating a new color.
			// So, create a new bitset.
			// boost::dynamic_bitset<> x(N);
			*(bitsets+maxgrp) = new boost::dynamic_bitset<>(N+1,0);
			// *(bitsets+maxgrp)->resize(N); 
            }

        //SDO_L50:
            color[jcol] = newColor;
            (*(bitsets+newColor))->set(jcol); 

            satDeg[jcol] = numord;
            numord++;

            // Termination Test.
            if(numord > N)
            {
                break;
            }

            // delete column jcol from the maxsat queue.
            deleteColumn(head,next,previous,maxsat,jcol);

            tag[jcol] = N;

            // Update the Saturation Degree for the Neighbors of
            // <id:jcol>

            for (int jp = jpntr[jcol] ; jp < jpntr[jcol+1] ; jp++)
            {
                int ir = row_ind[jp];

                for( int ip = ipntr[ir] ; ip < ipntr[ir+1] ; ip++)
                {
                    int ic = col_ind[ip];

                    if(tag[ic] < numord)
                    {
                        tag[ic] = numord;

                        // TODO: Eliminate this search using a bit vector. 
                        // bool isNewColor = true;

                        // // search the neighborhood of ic
                        // for (int x_jp = jpntr[ic]; x_jp < jpntr[ic+1] ; x_jp++)
                        // {
                        //     int x_ir = indRow[x_jp];
                        //     for (int x_ip = ipntr[ir]; x_ip < ipntr[ir+1]; x_ip++)
                        //     {
                        //         int x_ic = indCol[x_ip];
                        //         if(color[x_ic] == newColor)
                        //         {
                        //             isNewColor = false;
                        //             goto SDO_ISNEWCOLOR;
                        //         }
                        //     }
                        // }
                        // 
                        // // ========================================
                        // SDO_ISNEWCOLOR:
                        //bool isNewColor = (*(bitsets+newColor))->test(jcol); 
			bool isNewColor = (*(bitsets+newColor))->test(ic); 
                        if(!isNewColor)
                        {
			    (*(bitsets+newColor))->set(ic); 	
                            // update the pointers to the current saturation
                            // degree lists.
                            satDeg[ic]++;
                            // update the maxsat.
                            maxsat = max(maxsat,satDeg[ic]);

                            deleteColumn(head,next,previous,satDeg[ic]-1,ic);
                            addColumn(head,next,previous,satDeg[ic],ic);
                        }
			inducedDeg[ic] = inducedDeg[ic] - 1; 
                    }
                }
            }

        }
    }
    catch(std::bad_alloc)
    {
        std::cerr << "ERROR: Memory Exhausted " << std::endl;

        if(head) delete[] head;
        if(previous) delete[] previous;
        if(next) delete[] next;
        if(tag) delete[] tag;
        if(seqTag) delete[] seqTag;
        if(satDeg) delete[] satDeg;

       if(bitsets)
        {
	    /*	
            for(int i =0 ; i <= N; i++)
            {
                if(bitsets+i)
                {
                    delete (*(bitsets+i)); 
                } 
            }
	    */	
            delete[] bitsets; 
        } 
	
        return 0;
    }

    if(head) delete[] head;
    if(previous) delete[] previous;
    if(next) delete[] next;
    if(tag) delete[] tag;
    if(seqTag) delete[] seqTag;
    if(satDeg) delete[] satDeg;
    if(inducedDeg) delete[] inducedDeg;

    if(bitsets)
    {
        /*for(int i =0 ; i <= N; i++)
        {
            if(bitsets+i)
            {
                delete (*(bitsets+i)); 
            } 
        }
	*/	
        delete[] bitsets; 
    } 

    return maxgrp;
}
/* sdo() ENDS*/

/* prev sdo
int Matrix::sdo(int *color)
{
    int *satDeg = NULL;
    int *head = NULL;
    int *next = NULL;
    int *previous = NULL;
    int *tag = NULL;
    int *seqTag = NULL;
    int maxgrp = 0;

    try
    {
        // The following three integer arrays consist of a doubly linked satDeg. It acts
        // as a bucket priority queue for the incidence degree of the columns.

        // head(deg) is the first column in the deg satDeg unless head(deg) =
        // 0. If head(deg) = 0 there are no columns in the deg satDeg.

        // previous(col) is the column before col in the incidence satDeg unless
        // previous(col) = 0. If previous(col) = 0,  col is the first column in this
        // incidence satDeg.

        // next(col) is the column after col in the incidence satDeg unless
        // next(col) = 0. If next(col) = 0,  col is the last column in this incidence
        // satDeg.

        head     = new int[N];
        next     = new int[N+1];
        previous = new int[N+1];

        tag      = new int[N+1]; // for each unordered column, tag[jcol] stores
                                 // the number of order it has been processed for,
                                 // and for ordered/colored column, it stores N

        seqTag      = new int[N+1]; // This array of size n+1 is used for
                                    // searching the lowest possible color for a
                                    // column jcol.

        satDeg = new int[N+1]; // Array of size n+1, for each unordered column j,
                               // satDeg[j] is the saturation degree, where j =
                               // 1,2,3,...,n.
                               // For each ordered column j, satDeg[j] is the
                               // order in Staruation Degree Ordering.




        // Sort the indices of degree array <id:ndeg> in descending order, i.e
        // ndeg(tag(i)) is in descending order , i = 1,2,...,n
        //
        // <id:tag> is used here as an in-out-parameter to <id:indexSort> routine. It
        // will hold the sorted indices. The two arrays, <id:previous> and
        // <id:next> is used for temporary storage required for <id:indexSort>
        // routine.
        MatrixUtility::indexsort(N,N-1, ndeg,-1,tag , previous, next);

        // Initialize the doubly linked list, <id:satDeg>, and <id:tag> and <id:order> integer array.
        for (int jp = N; jp >= 1; jp--)
        {
            int ic = tag[jp]; //Tag is sorted indices for now 
            head[N-jp] = 0;

            addColumn(head,next,previous,0,ic);

            tag[jp] = 0;
            satDeg[jp] = 0;
            color[jp] = N;
            seqTag[jp] = 0;
        }

        int maximalClique = 0;
        int numord = 1;

        // determine the maximal search length to search for maximal degree in
        // the maximal incidence degree satDeg.
        int maxlst = 0;

        for( int ir = 1; ir <= M; ir++)
        {
            maxlst = maxlst + MatrixUtility::square(ipntr[ir+1] - ipntr[ir]);
        }

        maxlst = maxlst / N;

        int maxsat = 0;
        while(true)
        {
            int jp;
            int jcol;
            // Find a column jp with the maximal saturation degree.
            while(true)
            {
                jp = head[maxsat];
                if(jp > 0)
                    break;
                maxsat--;
            }

            // We search a distance of maxLast length to find the column with
            // maximal degree in the original graph.
            for(int numlst = 1,numwgt = -1;  numlst <= maxlst; numlst++)
            {
                if(ndeg[jp] > numwgt)
                {
                    numwgt = ndeg[jp];
                    jcol = jp;
                }
                jp = next[jp];
                if(jp <= 0)
                    break;
            }

            // To Color the column <id:jcol> with smallest possible number
            // we find all columns adjacent to column <id:jcol>.
            // and find the color that is not used yet.

            for(int jp = jpntr[jcol] ; jp < jpntr[jcol+1]  ; jp++)
            {
                int ir = row_ind[jp];

                for( int ip = ipntr[ir]; ip < ipntr[ir + 1] ; ip++)
                {
                    int ic = col_ind[ip];
                    seqTag[color[ic]] = jcol;
                }
            }

            int newColor;
            for (newColor = 1; newColor <= maxgrp; newColor++)
            {
                if(seqTag[newColor] != jcol)
                {
                	break;
                	//goto SDO_L50;
                }
            }
           //changed this to remove the goto statement
           if(newColor>maxgrp)
        	   maxgrp = maxgrp + 1;

        //SDO_L50:
            color[jcol] = newColor;

            satDeg[jcol] = numord;
            numord++;

            // Termination Test.
            if(numord > N)
            {
                break;
            }

            // delete column jcol from the maxsat queue.
            deleteColumn(head,next,previous,maxsat,jcol);

            tag[jcol] = N;

            // Update the Saturation Degree for the Neighbors of
            // <id:jcol>

            for (int jp = jpntr[jcol] ; jp < jpntr[jcol+1] ; jp++)
            {
                int ir = row_ind[jp];

                for( int ip = ipntr[ir] ; ip < ipntr[ir+1] ; ip++)
                {
                    int ic = col_ind[ip];

                    if(tag[ic] < numord)
                    {
                        tag[ic] = numord;

                        bool isNewColor = true;

                        // search the neighborhood of ic
                        for (int x_jp = jpntr[ic]; x_jp < jpntr[ic+1] ; x_jp++)
                        {
                            int x_ir = row_ind[x_jp];
                            for (int x_ip = ipntr[ir]; x_ip < ipntr[ir+1]; x_ip++)
                            {
                                int x_ic = col_ind[x_ip];
                                if(color[x_ic] == newColor)
                                {
                                    isNewColor = false;
                                    //goto SDO_ISNEWCOLOR;
                                    break;
                                }
                            }
                        }

                        // ========================================
                    //SDO_ISNEWCOLOR:
                        if(isNewColor)
                        {
                            // update the pointers to the current saturation
                            // degree lists.

                            satDeg[ic]++;
                            // update the maxsat.
                            maxsat = max(maxsat,satDeg[ic]);

                            deleteColumn(head,next,previous,satDeg[ic]-1,ic);
                            addColumn(head,next,previous,satDeg[ic],ic);
                        }
                    }
                }
            }

        }
    }
    catch(bad_alloc)
    {
        cerr << "ERROR: Memory Exhausted " << endl;

        if(head) delete[] head;
        if(previous) delete[] previous;
        if(next) delete[] next;
        if(tag) delete[] tag;
        if(seqTag) delete[] seqTag;

        return 0;
    }

    if(head) delete[] head;
    if(previous) delete[] previous;
    if(next) delete[] next;
    if(tag) delete[] tag;
    if(seqTag) delete[] seqTag;

    return maxgrp;
}
*/


int Matrix::updateDegreesToUVertices(int n, int jcol,int maxdeg, int *jpntr,int *row_ind,
                                     int *ipntr,int *col_ind, bool * inU, int *tag, int *u_tag,
                                     int *u_list, int *head, int *next, int *previous,int *list, int *blackList,const int q)
{
    // Update the degrees of the adjacent vertices.

    int numdeg;
    int ir;
    int ic;
    bool zero = false;

    for( int jp = jpntr[jcol] ; jp < jpntr[jcol+1] ; jp++)
    {
        ir = row_ind[jp];
        if(blackList[ir] == q)
            continue;
        for(int ip = ipntr[ir]; ip < ipntr[ir+1] ; ip++)
        {
            ic = col_ind[ip];

            if(inU[ic] == false && tag[ic] < n && u_tag[ic] != jcol)
            {
                u_tag[ic] = jcol;
                /**
                 * Update the pointers to the current degree u_lists.
                 */
                numdeg = u_list[ic];
                u_list[ic] = u_list[ic] + 1;
                maxdeg = max(numdeg + 1,maxdeg);

                deleteColumn(head,next,previous,numdeg,ic);
                addColumn(head,next,previous,numdeg+1,ic);

            }
        }
    }
    return maxdeg;
}

// This method produces a Seed matrix after the coloring has been done.
Matrix* Matrix::getSeedMatrix(int *ngrp)
{
    Matrix *m = new Matrix(N,numberOfColors,N, false);

    for (int i =1; i<= N; i++)
    {
        m->setIndRowEntry(i,i);
        m->setIndColEntry(i,ngrp[i]);
    }

    m->computeCCS();
    int nnz = m->compress();
    m->computeCRS();

    return m;
}

int Matrix::getNumberOfColors() const
{
    return numberOfColors;
}

