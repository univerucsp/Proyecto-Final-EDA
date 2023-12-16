#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

// Esta funcion obtiene una distancia euclediana
double distance(const std::vector<double> &vec1,
                const std::vector<double> &vec2) {
  if (vec1.size() != vec2.size()) {
    return -1.0;
  }
  double sum = 0.0;
  for (size_t i = 0; i < vec1.size(); ++i) {
    sum += pow(vec1[i] - vec2[i], 2);
  }
  return sqrt(sum);
}

// Estructura para almacenar los clusters en el preprocesamiento
class DisjointSet {

public:
  // Indice de padres
  std::vector<int> parent;

  DisjointSet(int n) {
    parent.resize(n);
    for (int i = 0; i < n; ++i) {
      parent[i] = i;
    }
  }

  // Hacemos que los disjoint set relacionados apunten al mismo padre
  int FindSet(int x) {
    if (parent[x] == x) {
      return x;
    }
    return parent[x] = FindSet(parent[x]);
  }

  // Funcion para unir disjoint sets
  void Union(int x, int y) {
    int x_root = FindSet(x);
    int y_root = FindSet(y);
    parent[x_root] = y_root;
  }
};

struct DataPoint {
  double x, y;
  vector<double> attributes;
};

struct Rectangle {
  double x1, y1, x2, y2;
  Rectangle() {
    x1 = 0;
    y1 = 0;
    x2 = 0;
    y2 = 0;
  }
  Rectangle(double X_MIN, double Y_MIN, double X_MAX, double Y_MAX) {
    x1 = X_MIN;
    y1 = Y_MIN;
    x2 = X_MAX;
    y2 = Y_MAX;
  }
  // Devuelve verdadero si el rectángulo actual se superpone parcialmente con el
  // rectángulo A
  bool HasOverlap(Rectangle A) {
    return !(x2 <= A.x1 || A.x2 <= x1 || y2 <= A.y1 || A.y2 <= y1);
  }

  // Devuelve verdadero si el rectángulo actual cubre completamente el
  // rectángulo A
  bool FullyContains(Rectangle A) {
    return x1 <= A.x1 && y1 <= A.y1 && x2 >= A.x2 && y2 >= A.y2;
  }

  bool Contains(double x, double y) {
    return x >= x1 && x <= x2 && y >= y1 && y <= y2;
  }
};

struct QuadtreeNode {
  QuadtreeNode(int point_start_index, int point_end_index, Rectangle area)
      : point_start_index(point_start_index), point_end_index(point_end_index),
        area(area) {}

  QuadtreeNode(Rectangle area)
      : point_start_index(-1), point_end_index(-1), area(area) {}

  // Indice inicial de los puntos
  int point_start_index;

  // Indice final de los puntos
  int point_end_index;

  Rectangle area;

  // Quadtree children
  std::vector<QuadtreeNode *> quadtree_children;
};

class QuadTree {
public:
  QuadTree(const std::vector<DataPoint> &input_points, double X_MIN,
           double Y_MIN, double X_MAX, double Y_MAX)
      : disjoint_query_result_(input_points.size()) {
    max_area = Rectangle(X_MIN, Y_MIN, X_MAX, Y_MAX);
    root_ = RecursiveConstruction(max_area, input_points);
    ComputeClusters(root_, 0);
  }

  std::vector<DataPoint> GetPoints(Rectangle query) {
    std::vector<DataPoint> result;
    GetPointsImpl(root_, query, result);
    return result;
  }

  std::vector<std::vector<DataPoint>> GetClusters(Rectangle query) {
    std::vector<std::pair<int, int>> covered_point_ranges;
    GetClustersImpl(root_, 0, query, covered_point_ranges);

    for (size_t i = 0; i < covered_point_ranges.size(); ++i) {
      for (size_t j = i + 1; j < covered_point_ranges.size(); ++j) {
        for (int p = covered_point_ranges[i].first;
             p < covered_point_ranges[i].second; ++p) {
          for (int q = covered_point_ranges[j].first;
               q < covered_point_ranges[j].second; ++q) {
            if (distance(points_[p].attributes, points_[q].attributes) <=
                kDbscanRadius) {
              disjoint_query_result_.Union(p, q);
            }
          }
        }
      }
    }

    std::unordered_map<int, std::vector<DataPoint>> result;
    for (size_t i = 0; i < covered_point_ranges.size(); ++i) {
      for (int p = covered_point_ranges[i].first;
           p < covered_point_ranges[i].second; ++p) {
        result[disjoint_query_result_.FindSet(p)].push_back(points_[p]);
      }
    }

    std::vector<std::vector<DataPoint>> final_result;
    for (const auto &cluster : result) {
      final_result.push_back(cluster.second);
    }

    return final_result;
  }

private:
  const double kDbscanRadius = 150;
  const int kMaxNumPointsPerLeaf = 137;

  int cluster_result_;
  Rectangle max_area;
  // Almacena un disjoint set por cada nivel del árbol
  vector<DisjointSet> disjoint_set_by_depth_;
  // Almacena los resultados del  disjoint set
  DisjointSet disjoint_query_result_;
  // Puntos guardados en el disjoint set
  vector<DataPoint> points_;
  QuadtreeNode *root_;

  QuadtreeNode *RecursiveConstruction(Rectangle area,
                                      const std::vector<DataPoint> &points) {
    if (points.empty()) {
      return nullptr;
    }

    if (points.size() < kMaxNumPointsPerLeaf) {
      QuadtreeNode *leaf_node = new QuadtreeNode(
          points_.size(), points_.size() + points.size(), area);
      points_.insert(points_.end(), points.begin(), points.end());
      return leaf_node;
    }

    QuadtreeNode *node = new QuadtreeNode(area);
    node->point_start_index = points_.size();

    // Divide el area actual en cuatro cuadrantes
    double mid_x = (area.x1 + area.x2) / 2.0;
    double mid_y = (area.y1 + area.y2) / 2.0;

    Rectangle sub_area1(area.x1, area.y1, mid_x, mid_y);
    Rectangle sub_area2(mid_x, area.y1, area.x2, mid_y);
    Rectangle sub_area3(area.x1, mid_y, mid_x, area.y2);
    Rectangle sub_area4(mid_x, mid_y, area.x2, area.y2);

    // Por cada subarea, obtienen puntos que caigan en esa region
    std::vector<DataPoint> subregion_points1 = GetSubRegion(sub_area1, points);
    std::vector<DataPoint> subregion_points2 = GetSubRegion(sub_area2, points);
    std::vector<DataPoint> subregion_points3 = GetSubRegion(sub_area3, points);
    std::vector<DataPoint> subregion_points4 = GetSubRegion(sub_area4, points);

    // Construye recursivamente quadtree para cada subarea
    node->quadtree_children.push_back(
        RecursiveConstruction(sub_area1, subregion_points1));
    node->quadtree_children.push_back(
        RecursiveConstruction(sub_area2, subregion_points2));
    node->quadtree_children.push_back(
        RecursiveConstruction(sub_area3, subregion_points3));
    node->quadtree_children.push_back(
        RecursiveConstruction(sub_area4, subregion_points4));

    node->point_end_index = points_.size();
    return node;
  }

  vector<DataPoint>
  GetSubRegion(Rectangle area, const std::vector<DataPoint> &point_references) {
    std::vector<DataPoint> result;
    for (const auto &point : point_references) {
      if (area.Contains(point.x, point.y)) {
        result.push_back(point);
      }
    }
    return result;
  }

  void GetPointsImpl(QuadtreeNode *node, Rectangle query,
                     std::vector<DataPoint> &result) {
    if (node == nullptr) {
      return;
    }

    if (!query.HasOverlap(node->area)) {
      return;
    }

    if (query.FullyContains(node->area)) {
      // Puntos de retorno en esta zona
      result.insert(result.end(), points_.begin() + node->point_start_index,
                    points_.begin() + node->point_end_index);
      return;
    }

    // Hoja, itera todos los puntos y comprueba
    if (node->quadtree_children.empty()) {
      for (int i = node->point_start_index; i < node->point_end_index; ++i) {
        if (query.Contains(points_[i].x, points_[i].y)) {
          result.push_back(points_[i]);
        }
      }
      return;
    }

    for (QuadtreeNode *children : node->quadtree_children) {
      GetPointsImpl(children, query, result);
    }
  }

  DisjointSet &GetDisjointSetByDepth(int depth) {
    while (disjoint_set_by_depth_.size() <= depth) {
      disjoint_set_by_depth_.push_back(DisjointSet(points_.size()));
    }
    return disjoint_set_by_depth_[depth];
  }

  void ComputeClusters(QuadtreeNode *node, int depth) {
    if (node == nullptr) {
      return;
    }
    //  Por un nodo hoja:
    if (node->point_end_index - node->point_start_index <
        kMaxNumPointsPerLeaf) {
      DisjointSet &disjoint_set = GetDisjointSetByDepth(depth);
      for (int i = node->point_start_index; i < node->point_end_index; ++i) {
        for (int j = i + 1; j < node->point_end_index; ++j) {
          if (distance(points_[i].attributes, points_[j].attributes) <=
              kDbscanRadius) {
            disjoint_set.Union(i, j);
          }
        }
      }
      return;
    }

    // Por nodos internos con children
    for (QuadtreeNode *children : node->quadtree_children) {
      ComputeClusters(children, depth + 1);
    }
    //  Reune disjoint set de children.
    std::vector<std::pair<int, int>> children_ranges;
    cout << "Profundidad Padre: " << depth << endl;
    DisjointSet &disjoint_set = GetDisjointSetByDepth(depth);
    for (QuadtreeNode *children : node->quadtree_children) {
      if (children == nullptr) {
        continue;
      }
      int point_children_start = children->point_start_index;
      int point_children_end = children->point_end_index;
      DisjointSet &children_disjoint_set = GetDisjointSetByDepth(depth + 1);
      std::cout << "Debug: start=" << point_children_start
                << ", end=" << point_children_end << std::endl;
      cout << "Hijo: " << children_disjoint_set.parent.size() << endl;
      cout << "Padre: " << disjoint_set.parent.size() << endl;
      std::copy(children_disjoint_set.parent.begin() + point_children_start,
                children_disjoint_set.parent.begin() + point_children_end,
                disjoint_set.parent.begin() + point_children_start);
      cout << "Pass" << endl;
      children_ranges.push_back(
          make_pair(point_children_start, point_children_end));
    }

    // Une disjoint sets segun la distancia de atributos
    for (size_t i = 0; i < children_ranges.size(); ++i) {
      for (size_t j = i + 1; j < children_ranges.size(); ++j) {
        for (int p = children_ranges[i].first; p < children_ranges[i].second;
             ++p) {
          for (int q = children_ranges[j].first; q < children_ranges[j].second;
               ++q) {
            if (distance(points_[p].attributes, points_[q].attributes) <=
                kDbscanRadius) {
              disjoint_set.Union(p, q);
            }
          }
        }
      }
    }
  }

  void GetClustersImpl(QuadtreeNode *node, int depth, Rectangle query,
                       std::vector<std::pair<int, int>> &covered_point_ranges) {
    if (node == nullptr) {
      return;
    }

    if (!query.HasOverlap(node->area)) {
      return;
    }

    DisjointSet &disjoint_set = GetDisjointSetByDepth(depth);

    if (query.FullyContains(node->area)) {
      covered_point_ranges.push_back(
          std::make_pair(node->point_start_index, node->point_end_index));

      std::copy(disjoint_set.parent.begin() + node->point_start_index,
                disjoint_set.parent.begin() + node->point_end_index,
                disjoint_query_result_.parent.begin() +
                    node->point_start_index);
      return;
    }

    // Hoja, itera todos los puntos y comprueba si pertenecen a la consulta
    if (node->quadtree_children.empty()) {
      for (int i = node->point_start_index; i < node->point_end_index; ++i) {
        if (query.Contains(points_[i].x, points_[i].y)) {
          covered_point_ranges.push_back(std::make_pair(i, i + 1));
          disjoint_query_result_.parent[i] = i;
        }
      }
      return;
    }

    for (QuadtreeNode *children : node->quadtree_children) {
      GetClustersImpl(children, depth, query, covered_point_ranges);
    }
  }
};

int main() {
  // Ruta del archivo
  std::string csv_file_path = "test_final.csv";
  // Abre el archivo
  std::ifstream csv_file(csv_file_path);
  if (!csv_file.is_open()) {
    std::cerr << "Error: Unable to open CSV file." << std::endl;
    return 1;
  }

  // Ignora la cabecera
  std::string header_line;
  std::getline(csv_file, header_line);
  // Lee puntos de muestra del archivo CSV con ';' como delimitador
  std::vector<DataPoint> sample_points;
  std::string line;
  while (std::getline(csv_file, line)) {
    std::istringstream iss(line);
    double index, x, y;
    char delimiter = ';';

    // Suponiendo que la primera columna es el índice y las dos columnas
    // siguientes son x y y
    if (!(iss >> index >> delimiter >> x >> delimiter >> y >> delimiter)) {
      std::cerr << "Error: Invalid CSV format." << std::endl;
      return 1;
    }

    // El resto de columnas son atributos
    std::vector<double> attributes;
    double attribute_value;
    while (iss >> attribute_value >> delimiter) {
      attributes.push_back(attribute_value);
    }

    // Anhadimos lo extraido a sample_points
    sample_points.push_back({x, y, attributes});

    // Mostramos todos los atributos para ver si se estan extrayendo
    // correctamente
    std::cout << "Attributes for DataPoint " << sample_points.size() << ": ";
    for (const auto &attribute : attributes) {
      std::cout << attribute << " ";
    }
    std::cout << std::endl;
  }

  // Cerramos el archivo
  csv_file.close();

  // Establecemos el rectángulo delimitador para QuadTree
  double X_MIN = -500.0, Y_MIN = -500.0, X_MAX = 500.0, Y_MAX = 500.0;

  // Creamos el quadtree
  QuadTree quad_tree(sample_points, X_MIN, Y_MIN, X_MAX, Y_MAX);
  cout << "Probando Extraccion de puntos" << endl;
  // Para mostrar que el quadtree es funcional extraemos los puntos para despues
  // mostrarlos de manera visual con visual.py
  Rectangle query_rect(-500, -500, 500, 500);
  vector<DataPoint> all_points = quad_tree.GetPoints(query_rect);
  // Guardamos todos los puntos en un archivo CSV
  std::ofstream all_points_file("all_points.csv");
  for (const auto &point : all_points) {
    all_points_file << point.x << "," << point.y << "\n";
  }
  all_points_file.close();
  cout << endl;
  double X1, Y1, X2, Y2;
  string cont = "si";
  while (cont != "no") {
    // Ingresamos el area de la cual queremos conocer los clusters
    cout << "Ingresa Area: " << endl;
    cout << "X1: ";
    cin >> X1;
    cout << "Y1: ";
    cin >> Y1;
    cout << "X2: ";
    cin >> X2;
    cout << "Y2: ";
    cin >> Y2;

    Rectangle query_cluster_rect(X1, Y1, X2, Y2);
    std::vector<std::vector<DataPoint>> clusters_in_query =
        quad_tree.GetClusters(query_cluster_rect);

    std::cout << "\nClusters dentro del rectangulo de consulta:" << std::endl;
    for (size_t i = 0; i < clusters_in_query.size(); ++i) {
      std::cout << "Cluster " << i + 1 << ":" << std::endl;
      for (const auto &point : clusters_in_query[i]) {
        std::cout << "(" << point.x << ", " << point.y << ") ,";
      }
      cout << endl;
    }

    // Abrir un solo archivo para todos los clusters
    std::ofstream all_clusters_file("all_clusters.csv");

    for (size_t i = 0; i < clusters_in_query.size(); ++i) {
      for (const auto &point : clusters_in_query[i]) {
        all_clusters_file << point.x << "," << point.y << "," << i << "\n";
      }
    }

    // Cerrar el archivo
    all_clusters_file.close();

    cout << "Deaseas hacer otra consulta: ";
    cin >> cont;
  }

  return 0;
}
