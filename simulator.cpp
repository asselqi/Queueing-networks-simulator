#include <iostream>
#include <random>
#include <chrono>
#include <fstream>
#include <vector>
//#include <string>


using std::vector;
using std::bernoulli_distribution;
using std::default_random_engine;
using std::exponential_distribution;
using std::stod;
using std::cout;
using std::endl;
using std::string;
std::random_device rndom;
std::mt19937 gen(rndom());
std::uniform_real_distribution<> dis(0, 1);


// The simulator's input
class Input {
public:
    double total_time;
    int num_of_stands;
    double lambda;
    double mui;
    vector<double> prob_vec;

	Input() = default;

    //constructor
    Input(int argc, char *argv[]) {
        int curr_index = 1;
        total_time = stod(string(argv[curr_index++]));
        num_of_stands = stod(string(argv[curr_index++]));
        lambda = stod(string(argv[curr_index++]));
        mui = stod(string(argv[curr_index++]));
        while (curr_index < argc) {
            prob_vec.push_back(stod(string(argv[curr_index++])));
        }
    }
};

class Stand {
public:
    vector<double> people_test_end_time;
    int tested_people;
    int left_people;
    double distr_param;
    double total_service_time;
    double total_wait_time;
    double total_last_time;
    double arrival_rate;
    double last_change_in_queue; // Time of last update in queue
    vector<double> prob_vec;
    vector<double> A_T;

    Stand(double distr_param, const vector<double> &prob_vec, int size) {
        people_test_end_time = vector<double>();
        tested_people = 0;
        left_people = 0;
        this->distr_param = distr_param;
        total_service_time = 0;
        total_wait_time = 0;
        total_last_time = 0;
        arrival_rate = 0;
        last_change_in_queue = 0;
        this->prob_vec = prob_vec;
        A_T = vector<double>(size, 0);
    }

    void arrived(double curr_time) {
        while(served(curr_time));
        bool stay = (dis(gen) <= prob_vec[people_test_end_time.size()]);
        if (stay && prob_vec[people_test_end_time.size()] != 0) {
            tested_people++;
            double next_event = (people_test_end_time.empty()) ? curr_time : people_test_end_time.back();
            double service_time = generator(distr_param);
            total_service_time += service_time;
            total_wait_time += next_event - curr_time;
            total_last_time = total_last_time > (next_event + service_time) ?
                              total_last_time : (next_event + service_time);
            people_test_end_time.push_back(next_event + service_time);
            arrival_rate = tested_people / curr_time;
        } else {
            left_people++;
        }
    }

    bool served(double curr_time) {
        if (people_test_end_time.empty()) {
            A_T[0] += curr_time - last_change_in_queue;
            last_change_in_queue = curr_time;
            return false;
        }
        vector<double>::iterator min_iterator = MinIndex();
        if (curr_time > (*min_iterator)) { // this customer is done, erase from queue.
            A_T[people_test_end_time.size()] += (*min_iterator - last_change_in_queue);
            last_change_in_queue = (*min_iterator);
            people_test_end_time.erase(min_iterator);
            return true;
        }
        else {
            A_T[people_test_end_time.size()] += (curr_time - last_change_in_queue);
            last_change_in_queue = curr_time;
            return false;
        }
    }

    vector<double>::iterator MinIndex() {
        std::vector<double>::iterator min_iterator = people_test_end_time.begin();
        for (std::vector<double>::iterator i = people_test_end_time.begin(); i != people_test_end_time.end(); ++i) {

            if (*i < *min_iterator) {
                min_iterator = i;
            }
        }
        return min_iterator;
    }

    vector<double>::iterator MaxIndex() {
        std::vector<double>::iterator min_iterator = people_test_end_time.begin();
        for (std::vector<double>::iterator i = people_test_end_time.begin(); i != people_test_end_time.end(); ++i) {

            if (*i > *min_iterator) {
                min_iterator = i;
            }
        }
        return min_iterator;
    }

    double generator(double distr_param) {
        int random = rand();
        random = (random == RAND_MAX) ? 0 : random;
        double u = (double) random / RAND_MAX;
        return log(1 - u) / (-distr_param);
    }
};

class System {
public:
	Input input;
    double total_wait_time;
    double total_serve_time;
    double total_last_time;
    double last_arrival_time;
    double last_leave_time;
    double curr_time;
    vector<Stand> stands;

    //output
    int Y;
    int X;
    double T_tag;
    vector<double> A_Ti;
    vector<double> Zi;
    double avg_wait_time;
    double avg_service_time;
    double avg_arrival_rate;
	
    System(int argc, char *argv[]) {
		input =  Input(argc, argv);
		//cout << input.num_of_stands << input.mui 
        total_wait_time = 0;
        total_serve_time = 0;
        total_last_time = 0;
        last_arrival_time = 0;
        last_leave_time = 0;
        curr_time = 0;
        Y = 0;
        X = 0;
        T_tag = 0;
        A_Ti = vector<double>(input.prob_vec.size(), 0);
        Zi = vector<double>(input.prob_vec.size(), 0);
        avg_wait_time = 0;
        avg_service_time = 0;
        avg_arrival_rate = 0;
        for (int i = 0; i < input.num_of_stands; ++i)
            stands.push_back(Stand(input.mui, input.prob_vec, input.prob_vec.size()));
    }

    double generator(double distr_param) {
        int random = rand();
        random = (random == RAND_MAX) ? 0 : random;
        double u = (double) random / RAND_MAX;
        return log(1 - u) / (-distr_param);
    }

    void simulate() {
        double arrival_time_off;
        int rand_stand;
        while(last_arrival_time <= input.total_time) {
            arrival_time_off = generator(input.lambda);
            last_arrival_time += arrival_time_off;
            rand_stand = (rand() % input.num_of_stands) + 1; //pick a random stand
            stands[rand_stand - 1].arrived(last_arrival_time);
        }
        for(std::vector<Stand>::iterator it = stands.begin(); it != stands.end(); ++it) {
            while ((*it).served(input.total_time));
        }

        for(std::vector<Stand>::iterator it = stands.begin(); it != stands.end(); ++it) {
            Y += (*it).tested_people;
            X += (*it).left_people;
            T_tag = T_tag > (*it).total_last_time ? T_tag : (*it).total_last_time;
            total_wait_time += (*it).total_wait_time / (*it).tested_people;
            total_serve_time += (*it).total_service_time / (*it).tested_people;
            for (int i = 0; i < (*it).A_T.size(); i++) {
                A_Ti[i] += (*it).A_T[i];
            }
        }

        avg_wait_time = total_wait_time / input.num_of_stands;
        avg_service_time = total_serve_time / input.num_of_stands;
        avg_arrival_rate = ((double)Y) / (T_tag * input.num_of_stands); 

        cout << Y << " ";
        cout << X << " ";
        cout << T_tag << " ";

        for(int i=0;i<A_Ti.size();i++){
            cout << A_Ti[i] / input.num_of_stands << " ";
        }
        for(int i=0;i<Zi.size();i++){
            cout << A_Ti[i] / (input.num_of_stands * T_tag) << " ";
        }
        cout << avg_wait_time << " " << avg_service_time << " " << avg_arrival_rate << endl;
    }
};

int main(int argc, char *argv[]) {
    System sys = System(argc,argv);
    sys.simulate();
    return 0;
}