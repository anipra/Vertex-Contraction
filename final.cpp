#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <utility>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/labeled_graph.hpp>

using namespace std;

struct myVertex{
   unsigned int vert;
};


int counter = 0;

typedef boost::labeled_graph<boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, myVertex>, unsigned int> UndirectedGraph;
typedef boost::graph_traits<UndirectedGraph>::edge_iterator EdgeIter;
typedef boost::graph_traits<UndirectedGraph>::vertex_iterator VertexIter;
typedef UndirectedGraph::vertex_descriptor Vertex;
typedef boost::graph_traits<UndirectedGraph>::adjacency_iterator adjIter;
typedef boost::graph_traits<UndirectedGraph>::out_edge_iterator OutEdgeIter;

typedef struct {
  UndirectedGraph graph;
  vector<pair<int, int>> max_planar_graph;
  vector<pair<int, int>> pairs_picked;
  vector<int> vertices;
  int number_vertices;
  set<pair<int, int>> list_collapsed;
}state;

void printAdjList(const UndirectedGraph& h)
{
   VertexIter vertex_i, vertex_end;
   cout <<"Inside printAdjList" << endl;
   OutEdgeIter out_i, out_end;
   tie(vertex_i, vertex_end) = vertices(h);
   for(; vertex_i != vertex_end; ++vertex_i)
   {
      unsigned int vertex = h.graph()[*vertex_i].vert;
      cout << vertex <<" is connected to: ";
      tie(out_i, out_end) = out_edges(*vertex_i, h);
      for(; out_i != out_end; ++out_i)
      {
         unsigned int target = boost::target(*out_i,h);
         cout << h.graph()[target].vert <<" ";
      }
      cout << endl;
   }
}

/* Tests for complete graph */
bool checkCompleteGraph(const UndirectedGraph& h)
{
   int num_verts = boost::num_vertices(h);
   VertexIter vertex_i, vertex_end;
   OutEdgeIter out_i, out_end;
   tie(vertex_i, vertex_end) = vertices(h);
   for(; vertex_i != vertex_end; ++vertex_i)
   {
      int count = 0;
      tie(out_i, out_end) = out_edges(*vertex_i, h);
      for(; out_i != out_end; ++out_i)
      {
         count++;
      }
      if(count != num_verts-1)
      {
         if(count != 0)
            return false;
      }
   }
   return true;
}

/* Finds the missing edges for the max planar graph and adds them to the set*/
void findMissingEdges(set<pair<int,int>>& missing_edges, const UndirectedGraph& h, const int& num_verts)
{
   vector<pair<int,int>> graph_vertices;
   VertexIter vertex_i, vertex_end;
   OutEdgeIter out_i, out_end;
   tie(vertex_i, vertex_end) = vertices(h);
   for(; vertex_i != vertex_end; ++vertex_i)
      graph_vertices.push_back(pair<int,int>(*vertex_i, h.graph()[*vertex_i].vert));
   tie(vertex_i, vertex_end) = vertices(h);
   for(; vertex_i != vertex_end; ++vertex_i)
   {  
      tie(out_i, out_end) = out_edges(*vertex_i, h);
      int count[num_verts];
      memset(count, 0, num_verts*sizeof(int));
      int value = h.graph()[*vertex_i].vert;
      count[*vertex_i] = 1; //set according to the indices
      for(; out_i != out_end; ++out_i)
      {
         unsigned int target_index = boost::target(*out_i,h);
         count[target_index] = 1; // set according to the indices 
      }
      for(int i = 0; i < num_verts; i++)
      {
         if(count[i] == 0)
         {
            int vertex_value = 0;
            for(auto it = graph_vertices.begin(); it != graph_vertices.end(); ++it)
            {
               if((*it).first == i)
               {
                  vertex_value = (*it).second;
                  break;
               }
            }
            if(vertex_value < value)
            {
               missing_edges.insert(make_pair(vertex_value, value));
            }
            else
            {
               missing_edges.insert(make_pair(value, vertex_value));
            }
         }
      }
   }
}

/* Adds the edges of the 2 chosen vertices into one */
void collapseAdjList(set<pair<int,int>>& collapsed_list, const UndirectedGraph& h, int src, int dest)
{
   VertexIter vertex_i, vertex_end;
   tie(vertex_i, vertex_end) = vertices(h);
   int src_index, dest_index;
   for(; vertex_i != vertex_end; ++vertex_i)
   {
      int value = h.graph()[*vertex_i].vert;
      if(value == src)
         src_index = *vertex_i;
      else if(value == dest)
         dest_index = *vertex_i;
   }
   int vertices[2] = {src_index, dest_index};
   OutEdgeIter out_i, out_end;
   for(int i = 0; i < 2; i++)
   {
      tie(out_i, out_end) = out_edges(vertices[i], h);
      for(; out_i != out_end; ++out_i)
      {
         int target_index = boost::target(*out_i, h);
         int value = h.graph()[target_index].vert;
         if(src > value)
            collapsed_list.insert(pair<int,int>(value, src));
         else
            collapsed_list.insert(pair<int,int>(src, value));
      }
   }
}

void duplicate_state(state& s, UndirectedGraph& h, vector<pair<int,int>>& mpg, vector<pair<int, int>>& track_pairs_picked, vector<int>& list_vertices, int& num_verts, set<pair<int, int>>& collapsed_list)
{
   s.graph = h;
   s.number_vertices = num_verts;
   for(auto it = list_vertices.begin(); it != list_vertices.end(); it++)
   {
      s.vertices.push_back(*it);
   }
   for(auto it = mpg.begin(); it != mpg.end(); it++)
   {
      s.max_planar_graph.push_back(pair<int, int>((*it).first, (*it).second));
   }
   for(auto it = track_pairs_picked.begin(); it != track_pairs_picked.end(); it++)
   {
      s.pairs_picked.push_back(pair<int, int>((*it).first, (*it).second));
   }
   for(auto it = collapsed_list.begin(); it != collapsed_list.end(); it++)
   {
      s.list_collapsed.insert(pair<int, int>((*it).first, (*it).second));
   }
}

/* Contracts the vertices till a complete graph is found */
int vertexContraction(UndirectedGraph h, vector<pair<int,int>> mpg, vector<pair<int, int>> track_pairs_picked, vector<int> list_vertices, int num_verts)
{
   set<pair<int,int>> missing_edges;
   set<pair<int,int>> collapsed_list;
   findMissingEdges(missing_edges, h, num_verts);

   //pick a pair
   int src, dest;
   for(auto it = missing_edges.begin(); it != missing_edges.end(); it++)
   {
      state s;
      duplicate_state(s, h, mpg, track_pairs_picked, list_vertices, num_verts, collapsed_list);
      src = (*it).first;
      dest = (*it).second;
      s.pairs_picked.push_back(pair<int,int>(src,dest));
 
      //deletes the vertices selected for contraction 
      for(vector<pair<int,int>>::iterator it_vec = s.max_planar_graph.begin(); it_vec != s.max_planar_graph.end();)
      {
         if((*it_vec).first == src || (*it_vec).first == dest || (*it_vec).second == src || (*it_vec).second == dest)
            it_vec = s.max_planar_graph.erase(it_vec);
         else
            it_vec++;
      }

      collapseAdjList(s.list_collapsed, s.graph, src, dest);
      for(auto it_vert = s.vertices.begin(); it_vert != s.vertices.end(); it_vert++)
      {
         if(*it_vert == dest)
         {
            it_vert = s.vertices.erase(it_vert);
            break;
         }
      }

      //pushes the contracted the edges in the vector
      for(auto it_vert = s.list_collapsed.begin(); it_vert != s.list_collapsed.end(); it_vert++)
      {
         s.max_planar_graph
         .push_back(pair<int, int>((*it_vert).first,(*it_vert).second));
      }

      s.number_vertices = num_verts-1;

      UndirectedGraph k;
      for(int i = 0; i < s.vertices.size(); ++i)
      {
         boost::add_vertex(s.vertices[i], k);
         k[s.vertices[i]].vert = s.vertices[i];
      }
      for(int i = 0; i < s.max_planar_graph.size(); i++)
      {
         boost::add_edge_by_label(s.max_planar_graph[i].first, s.max_planar_graph[i].second, k);
      }

      bool result = checkCompleteGraph(k);
      if(result == true)
      {
         counter++;
         cout << counter << " K" << s.number_vertices << " : ";
         for(int i = 0; i < s.pairs_picked.size(); i++)
         {
            cout << "(" << s.pairs_picked[i].first << "," << s.pairs_picked[i].second << ")" << " ";
         }
         cout << endl;
      }
      else
      {
         vertexContraction(k, s.max_planar_graph, s.pairs_picked, s.vertices, s.number_vertices);
      }
   }
}


int main(int argc, char* argv[])
{
   if(argc!=3)
   {
      cerr << "Usage: " << argv[0] << "num_verts max_planar_n_verts.txt" << endl;
      exit(1);
   }

   int num_verts = atoi(argv[1]);

   ifstream inf(argv[2]);
   if(!inf)
   {
      cerr << "Could not open max_planar_n_verts.txt" << endl;
   }

   vector<pair<int, int>> track_pairs_picked;  // store the list of vertices required to reach a complete graph 
   vector<vector<pair<int,int>>> list_planar_graphs; // stores all the vectors for graphs

   int src, dest, num_combo;
   if(!inf.good())
   {
      cerr << "Problem reading the maximal planar file." << endl;
   }
   inf >> num_combo;

   //calculate the number of edges
   int num_edges = 3*num_verts - 6;
   //populate the vectors and vector of vectors
   while(inf >> src >> dest)
   {
      vector<pair<int,int>> max_planar_graph; //vector for storing the edges read from the file
      max_planar_graph.push_back(pair<int,int>(src,dest));// push the first pair

      //push the remaining pairs
      for(int i = 0; i < num_edges-1; i++)
      {
         inf >> src >> dest;
         if(!inf.good())
         {
            cerr << "Problem reading the file." << endl;
         }
         max_planar_graph.push_back(pair<int,int>(src,dest));
      }
      list_planar_graphs.push_back(max_planar_graph);
   }
   for(int i = 0; i < list_planar_graphs.size(); i++)
   {
      UndirectedGraph h;
      vector<int> list_vertices;
      for(int i = 0; i < num_verts; i++)
      {
         boost::add_vertex(i, h);
         h[i].vert = i;
         list_vertices.push_back(i);
      }
      for(int j = 0; j < list_planar_graphs[i].size(); j++)
      {
         boost::add_edge_by_label(list_planar_graphs[i][j].first, list_planar_graphs[i][j].second, h);
      }
      //cout << "---------------------" << endl;
      vertexContraction(h, list_planar_graphs[i], track_pairs_picked, list_vertices, num_verts);
   }
}
