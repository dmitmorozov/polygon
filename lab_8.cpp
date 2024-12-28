#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <stack>
#include <set>
#include <chrono>
#include <limits>

//Структура для вершин графа
struct Node {
	std::string id;
	double lon;
	double lat;
	std::vector<std::pair<Node*, double>> neighbors;
};

//Структура для графа
struct Graph {
	std::unordered_map<std::string, Node*> nodes;
	
	//Парсинг графа
	void get_graph(const std::string& file_name) {
		std::string line;
		std::ifstream file(file_name);
		while (std::getline(file, line))
		{
			auto first_dl = line.find(':');

			//Координаты вершины
			std::string node = line.substr(0, first_dl);
			char comma;
			double lon1, lat1;
			std::istringstream node_stream(node);
			node_stream >> lon1 >> comma >> lat1;
			
			//Встречалась ли вершина
			Node* curr_node;
			if (nodes.find(node) != nodes.end()) {
				curr_node = nodes[node];
			}
			else {
				curr_node = new Node{ node, lon1, lat1 };
				nodes[node] = curr_node;
			}
			

			//Соседние вершины
			std::string all_neighbs = line.substr(first_dl + 1);
			int left = 0;
			while (left < all_neighbs.size()) {
				auto second_dl = all_neighbs.find(';', left);
				std::string neighb = all_neighbs.substr(left, second_dl);
				double lon2, lat2, weight2;
				std::istringstream neighb_stream(neighb);
				neighb_stream >> lon2 >> comma >> lat2 >> comma >> weight2;

				//Встречалась ли вершина
				Node* new_neighb;
				int first_comma = neighb.find(',');
				int second_comma = neighb.find(',', first_comma + 1);
				std::string neighb_id = neighb.substr(0, second_comma);
				if (nodes.find(neighb_id) != nodes.end()) {
					new_neighb = nodes[neighb_id];
				}
				else {
					new_neighb = new Node{ neighb_id, lon2, lat2 };
					nodes[neighb_id] = new_neighb;
				}
				//Был ли такой сосед
				bool is_new_neighb = true;
				for (int i = 0; i < curr_node->neighbors.size(); i++) {
					if (curr_node->neighbors[i].first->id == neighb_id) {
						is_new_neighb = false;
						curr_node->neighbors[i] = { new_neighb, weight2 };
					}
				}
				//Добавление соседа
				if (is_new_neighb) {
					curr_node->neighbors.push_back({ new_neighb, weight2 });
				}
				left += second_dl + 1;
			}
		}
	}
	//Ближайшая вершина по заданным координатам
	Node* find_closest(double lat, double lon) {
		double min_distance = std::numeric_limits<double>::max();
		Node* node_founded = nullptr;

		for (auto& pair : nodes) {
			Node* node = pair.second;
			double distance = std::sqrt(std::pow(node->lat - lat, 2) + std::pow(node->lon - lon, 2)); //Расстояние
			if (distance < min_distance) {  
				node_founded = node;
				min_distance = distance;
			}
		}
		return node_founded;
	}

	//Поиск в ширину
	std::unordered_map<std::string, double> bfs(Node* start) {
		std::queue<std::string> q;
		std::unordered_map<std::string, bool> visited;
		std::unordered_map<std::string, double> dist;
		q.push(start->id);
		visited[start->id] = true;
		dist[start->id] = 0;

		while (!q.empty()) {
			//Берём вершину из очереди
			std::string cur = q.front();
			q.pop();

			//Проход по соседним вершинам и обновление расстояний
			for (auto neighb : nodes[cur]->neighbors) {
				if (!visited[neighb.first->id]) {
					visited[neighb.first->id] = true;
					q.push(neighb.first->id);
				}
				if (dist[neighb.first->id] != 0) {
					dist[neighb.first->id] = std::min(dist[neighb.first->id], dist[cur] + 1);
				}
				else {
					dist[neighb.first->id] = dist[cur] + 1;
				}
			}
		}
		return dist;
	}

	//Поиск в глубину
	std::unordered_map<std::string, double> dfs(Node* start) {
		std::stack<std::string> st;
		std::unordered_map<std::string, bool> visited;
		std::unordered_map<std::string, double> dist;
		st.push(start->id);
		visited[start->id] = true;
		dist[start->id] = 0;

		while (!st.empty()) {
			//Берём вершину из стэка
			std::string cur = st.top();
			st.pop();

			//Проход по соседним вершинам и обновление расстояний
			for (auto neighb : nodes[cur]->neighbors) {
				if (!visited[neighb.first->id]) {
					visited[neighb.first->id] = true;
					st.push(neighb.first->id);
				}
				if (dist[neighb.first->id] != 0) {
					dist[neighb.first->id] = std::min(dist[neighb.first->id], dist[cur] + 1);
				}
				else {
					dist[neighb.first->id] = dist[cur] + 1;
				}
			}
		}
		return dist;
	}

	//Алгоритм Дейкстры
	std::unordered_map<std::string, double> dijkstra(Node* start) {
		std::unordered_map<std::string, double> dist;
		std::set<std::pair<double, Node*>> pq; // Приоритетная очередь
		for (auto i : nodes) {
			dist[i.first] = std::numeric_limits<double>::infinity();
		}
		dist[start->id] = 0;
		pq.insert({ 0, nodes[start->id] });

		while (!pq.empty()) {
			//Берём ближайщую из непосещенных вершин
			Node* cur = pq.begin()->second;
			pq.erase(pq.begin());

			//Проход по соседним вершинам
			for (auto neighb : cur->neighbors) {
				Node* v = neighb.first;
				double weight = neighb.second;
				if (dist[cur->id] + weight < dist[v->id]) {
					dist[v->id] = dist[cur->id] + weight; // Обновляем расстояние
					pq.insert({ dist[v->id], v });
				}
			}
		}
		return dist;
	}
	//Расстояние между двумя точками (Евклид)
	double heuristic(Node* a, Node* b) {
		double x = a->lon - b->lon;
		double y = a->lat - b->lat;
		return std::sqrt(x * x + y * y);
	}

	//Алгоритм А*
	std::vector<Node*> a_star(Node* start, Node* goal) {
		std::priority_queue<std::pair<double, Node*>, std::vector<std::pair<double, Node*>>, std::greater<>> openSet;
		std::unordered_map<Node*, Node*> cameFrom;
		std::unordered_map<Node*, double> g; //Обычное расстояние
		std::unordered_map<Node*, double> f; //С учётом эвристики

		for (auto& pair : nodes) {
			g[pair.second] = std::numeric_limits<double>::infinity();
			f[pair.second] = std::numeric_limits<double>::infinity();
		}
		g[start] = 0;
		f[start] = heuristic(start, goal);
		openSet.emplace(f[start], start);

		while (!openSet.empty()) {
			Node* currentNode = openSet.top().second;
			openSet.pop();
			//Дошли до конечной вершины
			if (currentNode->id == goal->id) {
				std::vector<Node*> path;
				while (currentNode != nullptr) {
					path.push_back(currentNode);
					currentNode = cameFrom[currentNode];
				}
				std::reverse(path.begin(), path.end());
				return path; // Возвращаем найденный путь
			}
			//Проход по соседям
			for (const auto& neighborPair : currentNode->neighbors) {
				Node* neighbor = neighborPair.first;
				double weight = neighborPair.second;

				double tentative_gScore = g[currentNode] + weight;
				if (tentative_gScore < g[neighbor]) { //Обновление расстояния
					cameFrom[neighbor] = currentNode;
					g[neighbor] = tentative_gScore;
					f[neighbor] = g[neighbor] + heuristic(neighbor, goal);

					openSet.emplace(f[neighbor], neighbor);
				}
			}
		}

		return {}; // Путь не найден
	}
};


int main() {
	Graph graph;
	graph.get_graph("spb_graph2.txt");
	std::cout << "Graph complete" << "\n";
	double lon_start = 59.85202, lat_start = 30.32306;
	double lon_goal = 59.956907, lat_goal = 30.308364;

	Node* start = graph.find_closest(lon_start, lat_start);
	Node* goal = graph.find_closest(lon_goal, lat_goal);

	//BFS
	auto start_bfs = std::chrono::high_resolution_clock::now();
	std::unordered_map<std::string, double> res_bfs = graph.bfs(start);
	auto stop_bfs = std::chrono::high_resolution_clock::now();
	auto duration_bfs = std::chrono::duration_cast<std::chrono::microseconds>(stop_bfs - start_bfs);
	std::cout << "BFS time:" << duration_bfs.count() << "\n";
	std::cout << "BFS res" << res_bfs[goal->id] << "\n";

	//DFS
	auto start_dfs = std::chrono::high_resolution_clock::now();
	std::unordered_map<std::string, double> res_dfs = graph.dfs(start);
	auto stop_dfs = std::chrono::high_resolution_clock::now();
	auto duration_dfs = std::chrono::duration_cast<std::chrono::microseconds>(stop_dfs - start_dfs);
	std::cout << "DFS time:" << duration_dfs.count() << "\n";
	std::cout << "DFS res" << res_dfs[goal->id] << "\n";

	//Dijkstra
	auto start_dijkstra = std::chrono::high_resolution_clock::now();
	std::unordered_map<std::string, double> res_dijkstra = graph.dijkstra(start);
	auto stop_dijkstra = std::chrono::high_resolution_clock::now();
	auto duration_dijkstra = std::chrono::duration_cast<std::chrono::microseconds>(stop_dijkstra - start_dijkstra);
	std::cout << "Dijkstra time:" << duration_dijkstra.count() << "\n";
	std::cout << "Dijkstra res" << res_dijkstra[goal->id] << "\n";

	//A*
	auto start_a_star = std::chrono::high_resolution_clock::now();
	std::vector<Node*> res_a_star = graph.a_star(start, goal);
	auto stop_a_star = std::chrono::high_resolution_clock::now();
	auto duration_a_star = std::chrono::duration_cast<std::chrono::microseconds>(stop_a_star - start_a_star);
	std::cout << "A* time:" << duration_a_star.count() << "\n";
	std::cout << "A* res" << res_a_star.size() << "\n";
	return 0;
}
