//
// Created by Yannic Lieder on 26.09.19.
//

#ifndef ANGULARARTGALLERYPROBLEM_AAGP_IP_SOLVER_H
#define ANGULARARTGALLERYPROBLEM_AAGP_IP_SOLVER_H

#include <algcplex/cplex.hpp>

#include "floodlight/floodlight.h"

#include "utils/profiling.h"

namespace AAGP {
    template <typename Kernel>
    static int get_linear_index(const std::vector<std::vector<Floodlight<Kernel>>> &floodlights, std::pair<int, int> &index)
    {
        int linear_index = 0;
        for (int current_vertex = 0; current_vertex < index.first; ++current_vertex)
        {
            linear_index += floodlights[current_vertex].size();
        }
        linear_index += index.second;

        return linear_index;
    };

    template <typename Kernel>
    static std::pair<int, int> get_2d_index(const std::vector<std::vector<Floodlight<Kernel>>> &floodlights, int linear_index)
    {
        int current_vertex = 0;
        int count = 0;

        while (count + floodlights[current_vertex].size() <= linear_index)
        {
            count += floodlights[current_vertex].size();
            ++current_vertex;
        }

        return std::make_pair(current_vertex, linear_index - count);

    }

    template <typename Kernel>
    class IPSolver {
    public:

            struct ResultType {
            std::vector<Floodlight<Kernel>> solution;
            double value;
            bool solved;

            struct {
                std::chrono::milliseconds add_obj_func;
                std::chrono::milliseconds add_cell_constraints;
                std::chrono::milliseconds solve;
            } time;
        };

        explicit IPSolver( std::vector<std::vector<std::pair<int, int>>> &cells,
                           std::vector<std::vector<Floodlight<Kernel>>> &floodlights,
                          const int num_floodlights,
                          const bool cpx_logging = true,
                          const bool minimize_angle = true) :
                _floodlights(&floodlights),
                _cells(&cells),
                _num_floodlights(num_floodlights),
                _num_cells(_cells->size()),
                _env(),
                _model(_env),
                _cplex(_model),
                _vars(_env, num_floodlights),
                _cpx_logging(cpx_logging),
                _callbacks(_env, *this){

            _cplex.setParam(IloCplex::Param::Threads, std::thread::hardware_concurrency());
            _cplex.setParam(IloCplex::ParallelMode, IloCplex::Opportunistic);

            if (!cpx_logging) _cplex.setOut(_env.getNullStream());

            IloNumExpr objective_expr(_env);
            add_obj_func = measure_time<std::chrono::milliseconds>([&]
            {
                for (int floodlight_index = 0; floodlight_index < num_floodlights; ++floodlight_index)
                {
                    auto index = get_2d_index(*_floodlights, floodlight_index);
                    if (minimize_angle)
                    {
                        objective_expr += (*_floodlights)[index.first][index.second].angle() * _vars[floodlight_index];
                    } else {
                        objective_expr += _vars[floodlight_index];
                    }
                }
            });

            add_cell_constraints = measure_time<std::chrono::milliseconds>([&] {
                for (int cell_index = 0; cell_index < _num_cells; ++cell_index) {
                    IloNumExpr left_side_expression(_env);

                    if (_cells->at(cell_index).empty())
                    {
                        std::cerr << "ERROR" << std::endl;
                    }
                    for (auto &c : _cells->at(cell_index)) {

                        left_side_expression += this->_vars[get_linear_index(*_floodlights, c)];
                    }

                    _model.add(left_side_expression >= 1);
                }
            });

            _model.add(IloMinimize(_env, objective_expr));

            objective_expr.end();
        }

        ~IPSolver() {
            //
            // cleans up all memory used by CPLEX
            //
            _env.end();
        }

        ResultType solve() {
            std::vector<Floodlight<Kernel>> solution;
            auto objective_value = -1.0;
            auto solved = false;

            solve_time = measure_time<std::chrono::milliseconds>([&] {
                if (_cplex.solve())
                {
                    for (unsigned long i = 0; i < _num_floodlights; ++i)
                    {
                        if (_cplex.getValue(_vars[i]) > 0.5)
                        {
                            auto index = get_2d_index(*_floodlights, i);
                            solution.push_back(_floodlights->at(index.first).at(index.second));
                        }
                    }
                    solved = true;
                    objective_value = _cplex.getObjValue();
                }
            });

            auto result = ResultType{solution, objective_value, solved};
            result.time.add_obj_func = add_obj_func;
            result.time.add_cell_constraints = add_cell_constraints;
            result.time.solve = solve_time;

            if (!result.solved) result.value = 0;

            return result;
            //return result_type{solution, objective_value, solved, {add_obj_func, add_cell_constraints, solve_time}}; //TODO
        }

    private:
        std::vector<std::vector<std::pair<int, int>>> *_cells;
        std::vector<std::vector<Floodlight<Kernel>>> *_floodlights;

        using size_t = std::size_t;

        typedef cpxhelper::callback_container<IPSolver> Callbacks;

        const size_t _num_floodlights;
        const size_t _num_cells;

        IloEnv _env;
        IloModel _model;
        IloCplex _cplex;
        IloBoolVarArray _vars;

        bool _cpx_logging;

        std::chrono::milliseconds add_obj_func;
        std::chrono::milliseconds add_cell_constraints;
        std::chrono::milliseconds solve_time;

        Callbacks _callbacks;
    };
}

#endif //ANGULARARTGALLERYPROBLEM_AAGP_IP_SOLVER_H
