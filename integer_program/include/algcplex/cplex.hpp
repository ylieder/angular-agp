#ifndef ALGCPLEX_CPLEX_HPP
#define ALGCPLEX_CPLEX_HPP

/* needed for some MAC versions of CPLEX */
#include <cstring>
#include <sstream>
#include <chrono>
#include <vector>
#include <atomic>
#include <iostream>
#include <thread>
#include <mutex>

#ifndef IL_STD
#define IL_STD 1
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wignored-attributes"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4512)
#endif

#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <boost/config.hpp>
#include <boost/optional.hpp>
#include <boost/iterator/iterator_facade.hpp>

#ifndef BOOST_NORETURN
#define WE_DEFINED_BOOST_NORETURN
#define BOOST_NORETURN
#endif

namespace cpxhelper {
    /* IloCplex::CutManagement has the following options:
     * - IloCplex::UseCutForce puts the cut into the relaxation; the cut stays there. This is useful for cuts that are hard to generate. (Default for lazy constraints).
     * - IloCplex::UseCutPurge puts the cut into the relaxation; it may be purged later on when CPLEX deems it unhelpful. This is useful for cuts that are cheap to generate. (Default for UserCuts).
     * - IloCplex::UseCutFilter treats the cut as if it was found by CPLEX builtin heuristics; it may not apply the cut at all if it is too dense, for example.
     */

    class constraint_callback {
    public:
        virtual void getValues(IloNumArray vals, IloNumVarArray vars) = 0;
        virtual void getValues(IloNumArray vals, IloIntVarArray vars) = 0;
        virtual IloConstraint add(IloConstraint constraint, boost::optional<IloCplex::CutManagement> management = boost::optional<IloCplex::CutManagement>{}) = 0;
        virtual IloEnv getEnv() const noexcept = 0;
    };

    class user_cut_callback : public IloCplex::UserCutCallbackI, public constraint_callback {
    public:
        using IloCplex::UserCutCallbackI::addLocal;
        using IloCplex::UserCutCallbackI::getValue;
        using IloCplex::CallbackI::abort;

        IloConstraint add(IloConstraint constraint, boost::optional<IloCplex::CutManagement> management = boost::optional<IloCplex::CutManagement>{}) override {
            return this->IloCplex::UserCutCallbackI::add(constraint, management ? *management : IloCplex::UseCutPurge);
        }

        IloEnv getEnv() const noexcept override {
            return this->IloCplex::UserCutCallbackI::getEnv();
        }

        void getValues(IloNumArray vals, IloNumVarArray vars) override {
            this->IloCplex::UserCutCallbackI::getValues(vals, vars);
        }

        void getValues(IloNumArray vals, IloIntVarArray vars) override {
            this->IloCplex::UserCutCallbackI::getValues(vals, vars);
        }

    protected:
        user_cut_callback(IloEnv env) : IloCplex::UserCutCallbackI(env) {}
        ~user_cut_callback() {}
    };

    class lazy_constraint_callback : public IloCplex::LazyConstraintCallbackI, public constraint_callback {
    public:
        using IloCplex::LazyConstraintCallbackI::getValue;
        using IloCplex::LazyConstraintCallbackI::getValues;
        using IloCplex::CallbackI::abort;

        IloConstraint add(IloConstraint constraint, boost::optional<IloCplex::CutManagement> management = boost::optional<IloCplex::CutManagement>{}) override {
            return this->IloCplex::LazyConstraintCallbackI::add(constraint, management ? *management : IloCplex::UseCutForce);
        }

        IloEnv getEnv() const noexcept override {
            return this->IloCplex::LazyConstraintCallbackI::getEnv();
        }

        void getValues(IloNumArray vals, IloNumVarArray vars) override {
            this->IloCplex::LazyConstraintCallbackI::getValues(vals, vars);
        }

        void getValues(IloNumArray vals, IloIntVarArray vars) override {
            this->IloCplex::LazyConstraintCallbackI::getValues(vals, vars);
        }

    protected:
        lazy_constraint_callback(IloEnv env) : IloCplex::LazyConstraintCallbackI(env) {}
        ~lazy_constraint_callback() {}
    };

    class branch_callback : public IloCplex::BranchCallbackI {
    public:
        using IloCplex::BranchCallbackI::getEnv;
        using IloCplex::BranchCallbackI::makeBranch;
        using IloCplex::BranchCallbackI::prune;
        using IloCplex::BranchCallbackI::isIntegerFeasible;
        using IloCplex::BranchCallbackI::getBranchType;
        using IloCplex::BranchCallbackI::getNbranches;
        using IloCplex::BranchCallbackI::getBranch;
        using IloCplex::BranchCallbackI::getNodeData;
        using IloCplex::BranchCallbackI::getValue;
        using IloCplex::BranchCallbackI::getValues;
        using IloCplex::CallbackI::abort;

    protected:
        branch_callback(IloEnv env) : IloCplex::BranchCallbackI(env) {}
        ~branch_callback() {}
    };

    class node_callback : public IloCplex::NodeCallbackI {
    protected:
        node_callback(IloEnv env) : IloCplex::NodeCallbackI(env) {}
        ~node_callback() {}
    };

    class incumbent_callback : public IloCplex::IncumbentCallbackI {
    public:
        using IloCplex::IncumbentCallbackI::reject;
        using IloCplex::IncumbentCallbackI::getValue;
        using IloCplex::IncumbentCallbackI::getObjValue;
        using IloCplex::IncumbentCallbackI::getNodeData;
        using IloCplex::CallbackI::abort;

    protected:
        incumbent_callback(IloEnv env) : IloCplex::IncumbentCallbackI(env) {}
        ~incumbent_callback() {}
    };

    class heuristic_callback : public IloCplex::HeuristicCallbackI {
    public:
        using IloCplex::CallbackI::abort;

    protected:
        heuristic_callback(IloEnv env) : IloCplex::HeuristicCallbackI(env) {}
        ~heuristic_callback() {}
    };

    template<typename CallingType>
    class callback_container {
    public:
        callback_container(IloEnv env, CallingType& calling) :
                M_env(env),
                M_cptr(&calling)
        {}

        void reset_env(IloEnv env);
        inline ~callback_container();
        callback_container(const callback_container&) = delete;
        callback_container &operator=(const callback_container&) = delete;
        inline callback_container(callback_container&&) = delete;
        inline callback_container &operator=(callback_container&&) = delete;

        std::chrono::microseconds total_time() const noexcept {
            return lazy_constraint_time() + user_cut_time() + branch_time() + node_time() + incumbent_time() + heuristic_time();
        }

        std::uint64_t total_calls() const noexcept {
            return lazy_constraint_calls() + user_cut_calls() + branch_calls() + node_calls() + incumbent_calls() + heuristic_calls();
        }

        void clear_time() noexcept {
            clear_lazy_constraint_time();
            clear_user_cut_time();
            clear_branch_time();
            clear_node_time();
            clear_incumbent_time();
            clear_heuristic_time();
        }

        void use(IloCplex& cplex);

    private:
        typedef std::chrono::steady_clock clock;
        typedef clock::time_point        tpoint;

        IloEnv        M_env;
        CallingType *M_cptr;

#define ADD_CALLBACK_TYPE(callback_name) \
		private: bool M_time_##callback_name = false;\
		         std::atomic<std::uint64_t> M_##callback_name##_calls{0};\
		         std::atomic<std::uint64_t> M_##callback_name##_time{0};\
		         IloCplex::Callback M_##callback_name##_callback;\
		         void (CallingType::*M_##callback_name##_cb)(callback_name##_callback*) = nullptr;\
		public:  bool measures_time_##callback_name () const noexcept { return M_time_##callback_name; }\
		         void set_measures_time_##callback_name(bool enable) noexcept { M_time_##callback_name; }\
		         std::chrono::microseconds callback_name##_time() const noexcept { return std::chrono::microseconds(M_##callback_name##_time.load()); }\
		         void clear_##callback_name##_time() noexcept { M_##callback_name##_time.store(0); M_##callback_name##_calls.store(0); }\
		         std::uint64_t callback_name##_calls() const noexcept { return M_##callback_name##_calls.load(); }\
		         void set_callback(void (CallingType::*callback_pointer)(callback_name##_callback*), bool enable_timing = false) {\
		             if(M_##callback_name##_callback.getImpl()) \
		                 M_##callback_name##_callback.end();\
		             M_##callback_name##_cb = callback_pointer;\
		             M_##callback_name##_callback = IloCplex::Callback(new (M_env) callback_name##_cb(M_env, *this));\
		             M_time_##callback_name = enable_timing;\
		         }\
		private: struct callback_name##_cb : public callback_name##_callback {\
			callback_name##_cb(IloEnv env, callback_container& c) : callback_name##_callback(env), container(c) {}\
			IloCplex::CallbackI *duplicateCallback() const override { return new (this->getEnv()) callback_name##_cb(this->getEnv(), container); }\
			void main() override {\
				assert(container.M_##callback_name##_cb != nullptr);\
				CallingType& ct = *container.M_cptr;\
				void (CallingType::*cbptr)(callback_name##_callback*) = container.M_##callback_name##_cb;\
				if(container.M_time_##callback_name) {\
					tpoint before = clock::now();\
					(ct.*cbptr)(this);\
					tpoint after = clock::now();\
					container.M_##callback_name##_time += std::chrono::duration_cast<std::chrono::microseconds>(after-before).count();\
					++container.M_##callback_name##_calls;\
				} else {(ct.*cbptr)(this);}\
			}\
			callback_container &container;\
		}

    ADD_CALLBACK_TYPE(lazy_constraint);
    ADD_CALLBACK_TYPE(user_cut);
    ADD_CALLBACK_TYPE(branch);
    ADD_CALLBACK_TYPE(node);
    ADD_CALLBACK_TYPE(incumbent);
    ADD_CALLBACK_TYPE(heuristic);
#undef ADD_CALLBACK_TYPE
    };

#define DESTROY_CALLBACK(callback_name) if(M_##callback_name##_callback.getImpl()) { M_##callback_name##_callback.end(); } (void)0
    template<typename CallingType> callback_container<CallingType>::~callback_container() {
        //DESTROY_CALLBACK(lazy_constraint);
        DESTROY_CALLBACK(user_cut);
        DESTROY_CALLBACK(branch);
        DESTROY_CALLBACK(node);
        DESTROY_CALLBACK(incumbent);
        DESTROY_CALLBACK(heuristic);
    }
#undef DESTROY_CALLBACK

#define REENV_CALLBACK(callback_name) if(M_##callback_name##_callback.getImpl()) { M_##callback_name##_callback = IloCplex::Callback(new (M_env) callback_name##_cb(M_env, *this)); } (void)0
    template<typename CallingType> void callback_container<CallingType>::reset_env(IloEnv env) {
        M_env = env;
        REENV_CALLBACK(lazy_constraint);
        REENV_CALLBACK(user_cut);
        REENV_CALLBACK(branch);
        REENV_CALLBACK(node);
        REENV_CALLBACK(incumbent);
        REENV_CALLBACK(heuristic);
    }
#undef REENV_CALLBACK

#define USE_CALLBACK(callback_name) if(M_##callback_name##_cb != nullptr) { cplex.use(M_##callback_name##_callback); } (void)0
    template<typename CallingType> void callback_container<CallingType>::use(IloCplex& cplex) {
        USE_CALLBACK(lazy_constraint);
        USE_CALLBACK(user_cut);
        USE_CALLBACK(branch);
        USE_CALLBACK(node);
        USE_CALLBACK(incumbent);
        USE_CALLBACK(heuristic);
    }
#undef USE_CALLBACK

    struct extractable_compare {
        bool operator()(const IloExtractable& e1, const IloExtractable& e2) const noexcept {
            return e1.getImpl() == e2.getImpl();
        }
    };

    struct extractable_hash {
        std::size_t operator()(const IloExtractable& e) const noexcept {
            const std::hash<IloExtractableI*> hasher{};
            return hasher(e.getImpl());
        }
    };

    BOOST_NORETURN inline void wrap_and_throw_cplex_exception(IloException& ex, const char* catching_file, const char* catching_func, unsigned catching_line) {
        std::ostringstream message;

        if(dynamic_cast<IloAlgorithm::CannotChangeException*>(&ex)) {
            IloAlgorithm::CannotChangeException &e = dynamic_cast<IloAlgorithm::CannotChangeException &>(ex);
            IloExtractableArray &extractables = e.getExtractables();

            message << "IloAlgorithm::CannotChangeException: " << ex.getMessage() << std::endl;

            message << "Extractables: ";
            for(std::size_t i = 0, s = extractables.getSize(); i < s; ++i) {
                message << extractables[i] << " ";
            }
            message << std::endl;
        } else if(dynamic_cast<IloAlgorithm::NotExtractedException*>(&ex)) {
            message << "IloAlgorithm::NotExtractedException: " << ex.getMessage() << std::endl;
        } else if(dynamic_cast<IloAlgorithm::CannotExtractException*>(&ex)) {
            message << "IloAlgorithm::CannotExtractException: " << ex.getMessage() << std::endl;
        } else {
            message << "IloException: " << ex.getMessage() << std::endl;
        }

        message << "Caught in function " << catching_func << ", in '" << catching_file << "':" << catching_line;
        throw std::runtime_error(message.str());
    }

    namespace impl {
        template<typename T, typename... Args> struct addresult_or_void {
        private:
            struct addresult_or_void_impl {
                void add(...) {}
                template<typename... Args2> auto add(Args2&&... args) -> decltype(std::declval<T&>().add(std::forward<Args2>(args)...)) {}
            };

        public:
            typedef decltype(std::declval<addresult_or_void_impl>().add(std::forward<Args>(std::declval<Args>())...)) type;
        };

        template<typename T, typename Arg> struct indexresult_or_void {
        private:
            struct indexresult_or_void_impl {
                void index(...) {}
                template<typename Arg2> auto index(Arg2&& arg) -> decltype(std::declval<T&>()[std::forward<Arg2>(arg)]) {}
            };

        public:
            typedef decltype(std::declval<indexresult_or_void_impl>().index(std::forward<Arg>(std::declval<Arg>()))) type;
        };

        template<typename Wrapped, bool EndElements> struct wrap {
        public:
            template<typename... Args> wrap(Args&&... args) :
                    M_wrapped(std::forward<Args>(args)...)
            {}

            wrap(wrap&& o) noexcept : M_wrapped(o.M_wrapped) {
                o.M_wrapped = Wrapped();
            }

            wrap &operator=(wrap&& o) noexcept {
                if(&o != this) {
                    P_check_destroy();
                    M_wrapped = o.M_wrapped;
                    o.M_wrapped = Wrapped();
                }
                return *this;
            }

            ~wrap() { P_check_destroy(); }

            Wrapped *operator->() noexcept { return &M_wrapped; }
            const Wrapped *operator->() const noexcept { return &M_wrapped; }

            operator Wrapped&() noexcept { return M_wrapped; }
            operator const Wrapped&() const noexcept { return M_wrapped; }

            Wrapped &operator*() noexcept { return M_wrapped; }
            const Wrapped &operator*() const noexcept { return M_wrapped; }

            template<typename... Args> typename addresult_or_void<Wrapped,Args...>::type add(Args&&... args) {
                return M_wrapped.add(std::forward<Args>(args)...);
            }

            template<typename Arg> typename indexresult_or_void<Wrapped,Arg>::type operator[](Arg&& arg) {
                return M_wrapped[std::forward<Arg>(arg)];
            }

        private:
            wrap(const wrap&) = delete;
            wrap &operator=(const wrap&) = delete;

            void P_check_destroy() noexcept {
                if(M_wrapped.getImpl()) {
                    P_call_end_elements<EndElements>();
                    M_wrapped.end();
                }
            }

            template<bool B> typename std::enable_if<B, void>::type P_call_end_elements() {
                M_wrapped.endElements();
            }

            template<bool B> typename std::enable_if<!B,void>::type P_call_end_elements() {}

            Wrapped M_wrapped;
        };
    }

    template<typename T> using wrap = impl::wrap<T,false>;
    template<typename T> using wrap_array = impl::wrap<T,true>;

    namespace impl {
        template<typename T, typename U> struct copy_const_ {
            typedef U type;
        };

        template<typename T, typename U> struct copy_const_<const T, U> {
            typedef const U type;
        };

        template<typename ArrayType> struct cplex_array_types_ {
            typedef typename std::remove_cv<ArrayType>::type array_type;
            typedef typename std::remove_cv<typename std::remove_reference<decltype(std::declval<ArrayType&>()[0])>::type>::type value_type;
            typedef typename copy_const_<ArrayType, value_type>::type facade_value_type;
            typedef decltype(std::declval<ArrayType&>()[0]) reference;
        };
    }

    template<typename ArrayType> class basic_cplex_array_iterator :
            public boost::iterator_facade<
                    basic_cplex_array_iterator<ArrayType>,
                    typename impl::cplex_array_types_<ArrayType>::facade_value_type,
                    std::random_access_iterator_tag,
                    typename impl::cplex_array_types_<ArrayType>::reference
            >
    {
    private:
        typedef boost::iterator_facade<
                basic_cplex_array_iterator<ArrayType>,
                typename impl::cplex_array_types_<ArrayType>::facade_value_type,
                std::random_access_iterator_tag,
                typename impl::cplex_array_types_<ArrayType>::reference
        > super_type;

        typedef impl::cplex_array_types_<ArrayType> array_types;
        friend boost::iterator_core_access;

    public:
        typedef typename super_type::value_type value_type;
        typedef typename super_type::reference reference;
        typedef typename super_type::pointer   pointer;
        typedef typename super_type::difference_type difference_type;

        basic_cplex_array_iterator() {}

        basic_cplex_array_iterator(typename array_types::array_type a, difference_type i) :
                M_array(a), M_index(i)
        {}

        template<typename OtherArrayType> basic_cplex_array_iterator(
                const basic_cplex_array_iterator<OtherArrayType>& other,
                typename std::enable_if<std::is_convertible<OtherArrayType, ArrayType>::value,int>::type = 0
        ) :
                M_array(other.M_array), M_index(other.M_index)
        {}

    private:
        reference dereference() const noexcept {
            typename array_types::array_type tmp(M_array);
            return tmp[M_index];
        }

        template<typename OtherArrayType>
        bool equal(const basic_cplex_array_iterator<OtherArrayType>& o) const noexcept
        {
            assert(static_cast<IloAny>(M_array.getImpl()) == static_cast<IloAny>(o.M_array.getImpl()));
            return M_index == o.M_index;
        }

        void increment() noexcept {
            ++M_index;
        }

        void decrement() noexcept {
            --M_index;
        }

        void advance(difference_type d) noexcept {
            M_index += d;
        }

        template<typename OtherArrayType>
        difference_type distance_to(const basic_cplex_array_iterator<OtherArrayType>& o) const noexcept
        {
            assert(static_cast<IloAny>(M_array.getImpl()) == static_cast<IloAny>(o.M_array.getImpl()));
            return o.M_index - M_index;
        }

        typename array_types::array_type M_array;
        difference_type M_index;
    };

    template<typename ArrayT> using cplex_array_iterator = basic_cplex_array_iterator<ArrayT>;
    template<typename ArrayT> using cplex_array_const_iterator = basic_cplex_array_iterator<const ArrayT>;

    namespace impl {
        template<typename U> std::true_type enable_if_cplex_array_fn_(const IloArray<U>&);
        std::false_type enable_if_cplex_array_fn_(...);

        template<typename Array, typename T> struct enable_if_cplex_array_ :
                std::enable_if<decltype(enable_if_cplex_array_fn_(std::declval<Array&>()))::value, T>
        {};
    }

    struct cplex_parameters {
        std::size_t num_threads{0};
        IloCplex::MIPEmphasisType mip_emphasis{IloCplex::MIPEmphasisBalanced};
        bool opportunistic_parallel{true};
        bool numerical_emphasis{false};
        std::ostream *output = &std::cout;
    };

    template<typename Derived> class basic_solver {
    public:
        basic_solver() :
                M_env(),
                M_model(M_env),
                M_cplex(M_model),
                M_callbacks(M_env, static_cast<Derived&>(*this))
        {
            set_cplex_parameters(Derived::default_parameters());
        }

        void set_num_threads(std::size_t nthreads = 0) noexcept {
            if(nthreads == 0)
                nthreads = std::thread::hardware_concurrency();

            M_cplex.setParam(IloCplex::Threads, static_cast<CPXINT>(nthreads));
        }

        void set_opportunistic_parallel(bool opp) noexcept {
            M_cplex.setParam(IloCplex::ParallelMode, opp ? IloCplex::Opportunistic : IloCplex::Deterministic);
        }

        void set_numerical_emphasis(bool nume) noexcept {
            M_cplex.setParam(IloCplex::NumericalEmphasis, static_cast<IloBool>(nume));
        }

        void set_mip_emphasis(IloCplex::MIPEmphasisType mip_emphasis) {
            M_cplex.setParam(IloCplex::MIPEmphasis, mip_emphasis);
        }

        void set_cplex_parameters(const cplex_parameters& parameters) {
            M_parameters = parameters;

            set_num_threads(parameters.num_threads);
            set_mip_emphasis(parameters.mip_emphasis);
            set_numerical_emphasis(parameters.numerical_emphasis);
            set_opportunistic_parallel(parameters.opportunistic_parallel);
            set_cplex_output(parameters.output);
        }

        void set_cplex_output(std::ostream *output = nullptr) {
            if(output) {
                M_cplex.setOut(*output);
                M_cplex.setError(*output);
                M_cplex.setWarning(*output);
            } else {
                M_cplex.setOut(env().getNullStream());
                M_cplex.setError(env().getNullStream());
                M_cplex.setWarning(env().getNullStream());
            }
        }

    protected:
        static cplex_parameters default_parameters() noexcept {
            return cplex_parameters{};
        }

        ~basic_solver() {}

        IloEnv env() const noexcept {
            return M_env;
        }

        IloModel model() const noexcept {
            return M_model;
        }

        IloCplex solver() const noexcept {
            return M_cplex;
        }

        void clear() noexcept {
            M_cplex.end();
            M_model.end();
            M_model = IloModel(env());
            M_cplex = IloCplex(M_model);
            M_added_callbacks = false;
            set_cplex_parameters(M_parameters);
        }

        void clear_env() noexcept {
            M_env->end();
            M_env = IloEnv();
            M_model = IloModel(env());
            M_cplex = IloCplex(M_model);
            callbacks().reset_env(M_env);
            M_added_callbacks = false;
            set_cplex_parameters(M_parameters);
        }

        bool solve(IloAlgorithm::Status *out_status = nullptr) {
            if(!M_added_callbacks) {
                M_callbacks.use(M_cplex);
                M_added_callbacks = true;
            }

            IloBool result = M_cplex.solve();
            if(out_status) {
                *out_status = M_cplex.getStatus();
            }
            return result;
        }

        callback_container<Derived>& callbacks() {
            return M_callbacks;
        }

        std::unique_lock<std::mutex> make_lock() const {
            return std::unique_lock<std::mutex>(M_callback_lock);
        }

    private:
        wrap<IloEnv> M_env;
        IloModel M_model;
        IloCplex M_cplex;
        callback_container<Derived> M_callbacks;
        cplex_parameters M_parameters;
        bool M_added_callbacks = false;
        mutable std::mutex M_callback_lock;
    };
}

template<typename ArrayType>
static inline typename cpxhelper::impl::enable_if_cplex_array_<ArrayType, cpxhelper::cplex_array_iterator<ArrayType>>::type
begin(ArrayType& a) noexcept
{
    return cpxhelper::cplex_array_iterator<ArrayType>(a, 0);
}

template<typename ArrayType>
static inline typename cpxhelper::impl::enable_if_cplex_array_<ArrayType, cpxhelper::cplex_array_iterator<ArrayType>>::type
end(ArrayType& a) noexcept
{
    return cpxhelper::cplex_array_iterator<ArrayType>(a, a.getSize());
}

template<typename ArrayType>
static inline typename cpxhelper::impl::enable_if_cplex_array_<ArrayType, cpxhelper::cplex_array_const_iterator<ArrayType>>::type
begin(const ArrayType& a) noexcept
{
    return cpxhelper::cplex_array_const_iterator<ArrayType>(a, 0);
}

template<typename ArrayType>
static inline typename cpxhelper::impl::enable_if_cplex_array_<ArrayType, cpxhelper::cplex_array_const_iterator<ArrayType>>::type
end(const ArrayType& a) noexcept
{
    return cpxhelper::cplex_array_const_iterator<ArrayType>(a, a.getSize());
}

template<typename ArrayType>
static inline typename cpxhelper::impl::enable_if_cplex_array_<ArrayType, cpxhelper::cplex_array_const_iterator<ArrayType>>::type
cbegin(const ArrayType& a) noexcept
{
    return cpxhelper::cplex_array_const_iterator<ArrayType>(a, 0);
}

template<typename ArrayType>
static inline typename cpxhelper::impl::enable_if_cplex_array_<ArrayType, cpxhelper::cplex_array_const_iterator<ArrayType>>::type
cend(const ArrayType& a) noexcept
{
    return cpxhelper::cplex_array_const_iterator<ArrayType>(a, a.getSize());
}

namespace cpxhelper {
    template<typename CplexArrayType> struct cplex_range {
        cplex_range(CplexArrayType& a) :
                M_array(&a)
        {}

        auto begin() const noexcept -> decltype(::begin(std::declval<CplexArrayType&>())) {
            return ::begin(*M_array);
        }

        auto end() const noexcept -> decltype(::end(std::declval<CplexArrayType&>())) {
            return ::end(*M_array);
        }

    private:
        CplexArrayType *M_array;
    };

    template<typename CplexArrayType> static inline cplex_range<CplexArrayType> range(CplexArrayType& array) noexcept {
        return cplex_range<CplexArrayType>(array);
    }

    template<typename CplexArrayType> static inline cplex_range<const CplexArrayType> range(const CplexArrayType& array) noexcept {
        return cplex_range<const CplexArrayType>(array);
    }

    template<typename CplexArrayType> static inline cplex_range<const CplexArrayType> crange(const CplexArrayType& array) noexcept {
        return cplex_range<const CplexArrayType>(array);
    }
}

#define WRAP_AND_THROW_CPLEX_EXCEPTION(ex) ::cpxhelper::wrap_and_throw_cplex_exception(ex, __FILE__, __PRETTY_FUNCTION__, __LINE__)

#ifdef WE_DEFINED_BOOST_NORETURN
#undef WE_DEFINED_BOOST_NORETURN
#undef BOOST_NORETURN
#endif

#endif //ALGCPLEX_CPLEX_HPP