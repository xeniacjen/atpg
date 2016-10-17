#include "clique_partition.h"

using namespace std; 

int CliquePartition::get_degree_of_a_node(int x, int nodesize, const CompGraph& local_compat, int* node_set)
{
  int j=CLIQUE_UNKNOWN, node_degree=CLIQUE_UNKNOWN;

  node_degree=0;
  for (j=0; j<nodesize; j++) /* compute node degrees */
    {
      if(node_set[j] != CLIQUE_UNKNOWN)
	{
	  if(x != j && local_compat.get(x, j))
	    node_degree+=1;  
	}
    }

  return node_degree;
}

int CliquePartition::select_new_node(const CompGraph& local_compat, int nodesize, int* node_set)
{
 /*    if a node with priority, then pick that node 
  *      else a node with highest degree
  *        if multiple nodes then pick a node
  *           with highest neighbor wt
  *             if multiple pick one randomly.   
  */
  int i=CLIQUE_UNKNOWN, j=CLIQUE_UNKNOWN;
  int** degrees;
  degrees = new int*[nodesize];
	
	for(int i = 0; i < nodesize; i++) //declare empty comp graph filled with junk data
		degrees[i] = new int[nodesize];
  int max_degree=CLIQUE_UNKNOWN;
  int curr_max_degree=CLIQUE_UNKNOWN;
  int curr_node_degree=CLIQUE_UNKNOWN;
  int index=CLIQUE_UNKNOWN, curr_node=CLIQUE_UNKNOWN;
  int max_curr_neighbors_wt=CLIQUE_UNKNOWN;
  int curr_neighbors_wt=CLIQUE_UNKNOWN;
  int max_node=CLIQUE_UNKNOWN;
 
  for (i=0; i<nodesize; i++) /* initialize the degrees matrix */ 
    {  /* i dimension = node degree */
      for (j=0; j<nodesize; j++)  
	{  /* j dimension = node with degree=i */
	  degrees[i][j]=CLIQUE_UNKNOWN;
	}
    }

  curr_max_degree=0;
  curr_node_degree=0;

  for (i=0; i<nodesize; i++)  /* for each node do */
    {
	if(node_set[i] != CLIQUE_UNKNOWN)  /* if the node is still in N */
	{
	  curr_node_degree= get_degree_of_a_node(i, nodesize, local_compat, node_set);

#ifdef DEBUG
	  printf(" node=%d curr_node_degree = %d \n", i, curr_node_degree);
#endif
	  if(curr_node_degree > curr_max_degree)
	    {
	      curr_max_degree = curr_node_degree;
	    }

	  /* append to a list of nodes with degree=curr_node_degree */ 
	  index=0; 
	  while(degrees[curr_node_degree][index] != CLIQUE_UNKNOWN) index++;
	  degrees[curr_node_degree][index] = i; /* register this node */
	}
    }

  /* for debugging purposes.. 
  for (i=0; i<nodesize; i++) 
    {  
      for (j=0; j<nodesize; j++)  
	{  
	  printf(" %d %d %d \n", i, j, degrees[i][j]);
	}
    }
 */
  if(degrees[curr_max_degree][1] == CLIQUE_UNKNOWN) /* only one max node */
    max_node = degrees[curr_max_degree][0];
  else
    if(degrees[curr_max_degree][1] != CLIQUE_UNKNOWN) /* multiple max nodes */
      {
	index=0;
	max_curr_neighbors_wt=0;

	for(index=0; index<nodesize; index++) 
         /* go through all nodes with curr_max_degree*/
	  {
	    if(degrees[curr_max_degree][index] != CLIQUE_UNKNOWN) 
            /* not end of list of the nodes with curr_max_degree */
	      { 
		curr_neighbors_wt=0;
		curr_node = degrees[curr_max_degree][index];

		/* get cumulative neighbor weight for this node */
		curr_neighbors_wt += get_degree_of_a_node(curr_node, nodesize,
							  local_compat, node_set);
#ifdef DEBUG
		printf("curr_node = %d curr_neighbors_wt=%d\n", curr_node, curr_neighbors_wt);
#endif
		/* Is the local_compat, node_set consistent? */
		if(curr_neighbors_wt >= max_curr_neighbors_wt)
		  { 
		    max_curr_neighbors_wt = curr_neighbors_wt;
		    max_node = curr_node;
		  }
	      }
	  }
      }
#ifdef DEBUG
  printf(" curr_max_degree = %d max_node= %d\n", curr_max_degree, max_node);
#endif

  for(int i = 0; i < nodesize; i++) //declare empty comp graph filled with junk data
	free(degrees[i]); 
  free(degrees); 

  return max_node;
}

int CliquePartition::form_setY(int* setY, int* current_clique, const CompGraph& local_compat, int nodesize, int* node_set)
{
  int i=CLIQUE_UNKNOWN, j=CLIQUE_UNKNOWN, index=CLIQUE_UNKNOWN;
  int setY_size = CLIQUE_UNKNOWN;
  int compatibility=CLIQUE_UNKNOWN;
  
  index=0;
  setY_size=0;

  /* reset set_Y */
  for(i=0; i<nodesize; i++) 
  {
      setY[i]=CLIQUE_UNKNOWN;
  }

  for(i=0; i<nodesize; i++)
    {
      compatibility=CLIQUE_TRUE;
      if(node_set[i] != CLIQUE_UNKNOWN) 
	{
	  for(j=0; j<nodesize; j++) 
	    {
	      if(current_clique[j] != CLIQUE_UNKNOWN)
	      {
		if(!local_compat.get(current_clique[j], i))
		  {
		    compatibility=CLIQUE_FALSE;
		    break;
		  }
	      }
	      else
		break;
	    }
	  if(compatibility == CLIQUE_TRUE)
	    {
	      setY[index]=i;
	      index++;
	    }
	}
    }
  setY_size = index;

  return setY_size;
}

void CliquePartition::print_setY(int* setY)
{
  int index=CLIQUE_UNKNOWN;

  index=0;
  printf (" setY = {");
  while(setY[index] != CLIQUE_UNKNOWN)
    {
      printf(" %d ", setY[index]);
      index++;
    }
  printf("}\n"); 
}

void CliquePartition::form_set_Y1(int nodesize, int* set_Y, int* set_Y1, int** sets_I_y, int* node_set)
{
  int i=CLIQUE_UNKNOWN, j=CLIQUE_UNKNOWN, k=CLIQUE_UNKNOWN;
  int* cards=(int*)NULL;
  int min_val = CLIQUE_UNKNOWN;
  int curr_index =CLIQUE_UNKNOWN;
  int curr_y = CLIQUE_UNKNOWN;
  
  cards=(int*) malloc (nodesize * sizeof(int));

  for(i=0; i<nodesize; i++) 
    { 
      set_Y1[i]=CLIQUE_UNKNOWN;
      cards[i]=0;
    }

  /* Get the cardinalities of  intersection(I_y, setY) 
     for each y in I_y */
  for(i=0; i<nodesize; i++) /* for each y in I_y */
    {
      if(set_Y[i] != CLIQUE_UNKNOWN)  
      {
	  curr_y = set_Y[i];
	  
	  for(j=0; j<nodesize; j++) /* for each node in I_y of curr_y*/
	  {  
	      if(sets_I_y[curr_y][j] != CLIQUE_UNKNOWN) 
	      {
		  for(k=0; k<nodesize; k++) /* for each node in set_Y */
		  { 
		      if(set_Y[k] != CLIQUE_UNKNOWN) 
		      {
			  if(sets_I_y[i][j] == set_Y[k])
			      cards[i]++;
		      }
		      else
			  break; /* end of set_Y */
		  }
	      }
	      else
		  break;
	  }
      }
    }

  min_val=cards[0];
  for(i=0; i<nodesize; i++)
    {
	if(set_Y[i] != CLIQUE_UNKNOWN) 
	{
	    if(cards[i] < min_val)
		min_val=cards[i];
	}
    }

#ifdef DEBUG
  printf(" min_val = %d ", min_val); 
#endif

  curr_index = 0;
  for(i=0; i<nodesize; i++)
    {
      if(cards[i] == min_val)
	{
	  set_Y1[curr_index] = set_Y[i];
	  curr_index++;
	}
    }

#ifdef DEBUG
  printf(" Set Y1 = { ");
  for(i=0; i<nodesize; i++)
    {
	if(set_Y1[i] !=CLIQUE_UNKNOWN)	
	    printf(" %d ", set_Y1[i]);
    }
  printf(" }\n");
#endif

  free(cards); 
  return;
}

void CliquePartition::form_set_Y2(int nodesize, int* set_Y2, int* set_Y1, int* sizes_of_sets_I_y)
{
  int i=CLIQUE_UNKNOWN;
  int max_val=CLIQUE_UNKNOWN;
  int curr_index=CLIQUE_UNKNOWN;

  for(i=0; i<nodesize; i++)
    {
      if(set_Y1[i] != CLIQUE_UNKNOWN)
	{
	    
	  if(sizes_of_sets_I_y[ set_Y1[i] ] > max_val)
	    {
	      max_val = sizes_of_sets_I_y[ set_Y1[i] ]  ;
	    }
	}
      else
	break;
    }

  curr_index=0;
  for(i=0; i<nodesize; i++)
    {
      if(set_Y1[i] != CLIQUE_UNKNOWN)
	{
	    
	  if(sizes_of_sets_I_y[ set_Y1[i] ] == max_val)
	    {
	      set_Y2[curr_index]=set_Y1[i];
	      curr_index++;
	    }
	}
      else
	break;
    }

#ifdef DEBUG
  printf(" curr_index = %d   max_val = %d ", curr_index, max_val);
  printf(" Set Y2 = { ");
  for(i=0; i<nodesize; i++)
    {
	if(set_Y2[i] != CLIQUE_UNKNOWN) 
	{
	    printf(" %d ", set_Y2[i]);
	}
	else
	    break;
    }
  printf(" }\n");
#endif

  return;
}

int CliquePartition::pick_a_node_to_merge(int* setY, const CompGraph& local_compat, int* node_set, int nodesize)
{
  int** sets_I_y=(int**) NULL;
  int i=CLIQUE_UNKNOWN, j=CLIQUE_UNKNOWN;
  int* curr_indexes=(int*) NULL;
  int* set_Y1=(int*) NULL;
  int* set_Y2=(int*) NULL;
  int* sizes_of_sets_I_y=(int*) NULL;
  int min_val=CLIQUE_UNKNOWN;
  int new_node=CLIQUE_UNKNOWN;
  int curr_node_in_setY =CLIQUE_UNKNOWN;
  
  /* dynamically allocate memory for sets_I_y array */

  sets_I_y=(int**) malloc (nodesize * sizeof(int*));

  for(i=0; i<nodesize; i++)
    {
      sets_I_y[i] = (int*) malloc (nodesize * sizeof(int));
    }

  curr_indexes=(int*) malloc (nodesize * sizeof(int));
  sizes_of_sets_I_y=(int*) malloc (nodesize * sizeof(int));

  for(i=0; i<nodesize; i++) 
    {
      curr_indexes[i]=0;
      sizes_of_sets_I_y[i]=0;
      for(j=0; j<nodesize; j++) 
	{
	  sets_I_y[i][j]=CLIQUE_UNKNOWN;
	}
    }

  /* form I_y sets */

  for(i=0; i<nodesize; i++) 
    {  
      if(setY[i] != CLIQUE_UNKNOWN) /* for each y in Y do */
	{ 
	  for(j=0; j<nodesize; j++) 
	    {
	      if(node_set[j] != CLIQUE_UNKNOWN)
		{  /* if this node is still in set N */
		  if(!local_compat.get(setY[i], j)) 
		    { 
		      sets_I_y[setY[i]][curr_indexes[setY[i]]]=j;
		      curr_indexes[setY[i]]++;
		    }
		}
	    }
	}
      else
	break;  /* end of setY */
    }

  for(i=0; i<nodesize; i++) 
    { 
      if(setY[i] != CLIQUE_UNKNOWN) /* for each y in Y do */
	{ 
	    curr_node_in_setY = setY[i];

	    /* copy curr index into sizes */
	    sizes_of_sets_I_y[curr_node_in_setY] = curr_indexes[curr_node_in_setY];
	    
	    /* print all I_y sets */

#ifdef DEBUG
	    printf(" i= %d  nodeno= %d, curr_index = %d  ", i, curr_node_in_setY, curr_indexes[curr_node_in_setY]); 

	    print_setY(sets_I_y[curr_node_in_setY]);
#endif
	}
    }

  /* form set_Y1 */
  set_Y1=(int*) malloc (nodesize * sizeof(int));
  for(i=0; i<nodesize; i++) {set_Y1[i] = CLIQUE_UNKNOWN;}
  form_set_Y1(nodesize, setY, set_Y1, sets_I_y, node_set); 

  /* form set_Y2 */
  set_Y2=(int*) malloc (nodesize * sizeof(int));
  for(i=0; i<nodesize; i++) {set_Y2[i] = CLIQUE_UNKNOWN;}
  form_set_Y2(nodesize, set_Y2, set_Y1, sizes_of_sets_I_y); 

  if(set_Y2[0] != CLIQUE_UNKNOWN)
    new_node = set_Y2[0];
  
  for(i=0; i<nodesize; i++)
    {
      free(sets_I_y[i]); 
    }
  free(sets_I_y); 
  free(curr_indexes); 
  free(set_Y1); 
  free(set_Y2); 
  free(sizes_of_sets_I_y); 

  return new_node; 
}

void CliquePartition::print_clique_set()
{
    int i=UNKNOWN, j=UNKNOWN;

    printf("\n Clique Set: \n");
    
    for(i=0; i<(int)clique_set.size(); i++)
    {
	printf("\tClique #%d (size = %d) = { ",i, clique_set[i].size);
	
	for(j=0; j<(int)clique_set[i].members.size(); j++)
	{
		printf(" %d ", clique_set[i].members[j]);
	}
	printf (" }\n");
    }
    printf("\n");
}

int CliquePartition::clique_partition(const CompGraph& compat, int nodesize)
{
  const CompGraph& local_compat = compat; 
  int* current_clique=(int*) NULL; 
  int* node_set=(int*) NULL; 
  int* setY=(int*) NULL; 
  int i=CLIQUE_UNKNOWN, j=CLIQUE_UNKNOWN;
  int node_x=CLIQUE_UNKNOWN, node_y=CLIQUE_UNKNOWN;
  int setY_cardinality=CLIQUE_UNKNOWN;
  int new_node=CLIQUE_UNKNOWN;
  int curr_index=CLIQUE_UNKNOWN;
  int size_N = CLIQUE_UNKNOWN;
  /*int nodesize=CLIQUE_UNKNOWN;*/

#ifdef DEBUG
  printf("\n");
  printf("**************************************\n");
  printf(" *       Clique Partitioner         *\n");
  printf("**************************************\n");
  printf("\nEntering Clique Partitioner.. \n");

  printf(" You entered the compatibility array: \n");

  for(i=0; i<nodesize; i++) 
    {
	printf("\t");
      for(j=0; j<nodesize; j++) 
	{
	  printf("%d ",local_compat.get(i, j));
	}
      printf("\n");
    }
#endif 

  /* allocate memory for current clique & initialize to unknown values*/
  /* - current_clique has the indices of nodes that are compatible with each other*/
  /* - A node i is in node_set if node_set[i] = i */ 

  current_clique = (int*) malloc (nodesize * sizeof(int)); 
  node_set = (int*) malloc (nodesize * sizeof(int));
  setY = (int*) malloc (nodesize * sizeof(int));

  for(i=0; i<nodesize; i++) 
    {
      current_clique[i]=CLIQUE_UNKNOWN;
      node_set[i]=i;
      setY[i] = CLIQUE_UNKNOWN;
    }

  size_N = nodesize;
  curr_index = 0; /* reset the index to start for current clique */

  while(size_N > 0) /* i.e still cliques to be formed */
    {

#ifdef DEBUG
      printf( "=====================================================\n");
      printf(" size_N = %d  node_set = { ", size_N);
      for(i=0; i<nodesize; i++) { 
	  printf (" %d ", node_set[i]);
      }
      printf(" }\n");
#endif
      
      if(current_clique[0] == CLIQUE_UNKNOWN)  /* new clique formation */
	{ 
	  node_x = select_new_node(local_compat, nodesize, node_set);
#ifdef DEBUG
	  printf(" Node x = %d\n", node_x);   /* first node in the clique */
#endif
	  current_clique[curr_index] = node_x; 
	  node_set[node_x] = CLIQUE_UNKNOWN;   /* remove node_x from N i.e node_set */
	  curr_index++;
	}

      setY_cardinality=CLIQUE_UNKNOWN;
      setY_cardinality=form_setY(setY, current_clique, local_compat, 
				 nodesize, node_set); 
#ifdef DEBUG
      print_setY(setY);
	  printf (" Set Y cardinality = %d \n", setY_cardinality);
#endif
      

      if(setY_cardinality == 0) /* No possible nodes for merger; declare current_cliqueas complete */
	{   //printf("completing clique!\n");
          /* copy the current clique into central datastructure */
        clique c; 
	    c.size = 0;

	    //printf(" A clique is found!! Clique = { ");
		
	    for(i=0; i< nodesize; i++) 
	    {
		//printf("node_index = %d\n",i );
		if(current_clique[i] != CLIQUE_UNKNOWN) 
		{
		    c.members.push_back(current_clique[i]);
		    
		    //printf(" %d ", current_clique[i]);
			
		    
			
		    node_set[current_clique[i]]=CLIQUE_UNKNOWN; /* remove this node from the node list */
			current_clique[i]=CLIQUE_UNKNOWN;
		    size_N = (size_N - 1);
		}
		else
		{
            c.size = (int) c.members.size();  
            if (c.size>0)
                clique_set.push_back(c); 
		    break;
		}
	    }
	    //printf(" }\n");
	    curr_index = 0; /* reset the curr_index for the next clique */
	}
      else
	{
	  node_y=pick_a_node_to_merge(setY, local_compat, node_set, nodesize);
	  current_clique[curr_index] = node_y;
	  node_set[node_y] = CLIQUE_UNKNOWN;
#ifdef DEBUG
	  printf(" y (new node) = %d \n", node_y);
#endif
	  curr_index++;
	}
    }
#ifdef DEBUG
  printf("\n Final Clique Partitioning Results:\n");
  print_clique_set();
  printf("Exiting Clique Partitioner.. Bye.\n");
  printf("**************************************\n\n");
#endif 

  free(current_clique); 
  free(node_set); 
  free(setY); 

  return 1;
}
