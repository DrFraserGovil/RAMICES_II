#pragma once 
#include <functional>

/*!
    FEAR YE WHO ENTER HERE

    This code is a bunch of awful, awful template metaprogramming

    It is designed to allow the parallel code to perform some compile time checks. 

    The end goal is the `COMPILE_TIME_REFERENCE_CHECKER'. This checks that the function which is being performed in parallel has the suitable number of of arguments being passed to it, and so on.

    This was written with the aid of an LLM, and so might be totally awful and inefficient. 
*/


// Helper trait to detect if a type is a std::reference_wrapper
template<typename T>
struct is_reference_wrapper : std::false_type {};

template<typename T>
struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {};

template<typename T>
constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

// --- Function Traits to extract Callable's parameter types ---
// General template for function traits (will be specialized below)
template <typename T>
struct function_traits;

// Specialization for function pointers
template <typename R, typename... Args>
struct function_traits<R (*)(Args...)> {
    static constexpr size_t arity = sizeof...(Args);
    using return_type = R;
    template <size_t I>
    using arg_type = typename std::tuple_element<I, std::tuple<Args...>>::type;
};

// Specialization for std::function (or other callable objects like lambdas in C++11/14)
// This is a common pattern for lambdas, but works for other classes with operator()
template <typename T>
struct function_traits : function_traits<decltype(&T::operator())> {};

// Specialization for mutable lambdas/callable objects
template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...)> : function_traits<R (*)(Args...)> {};

// Specialization for const lambdas/callable objects
template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const> : function_traits<R (*)(Args...)> {};

#define COMPILE_TIME_REFERENCE_CATCHER(inserted_count, CallableType, ArgsTypes) \
static_assert(sizeof...(ArgsTypes) >= 0, "Dummy expansion for Intellisense"); /* Dummy usage */ \
    /* First, a sanity check: ensure number of args passed matches callable's arity */ \
    static_assert(function_traits<CallableType>::arity == (inserted_count + sizeof...(ArgsTypes)), \
        "\n\n\tError: The number of arguments passed to Task/For does not match the callable's expected arity.\n\n"); \
\
    /* Helper lambda to check compliance for a single argument at compile time */ \
    auto check_arg_compliance_lambda = []<size_t I>() { \
        /* Type of the argument as passed to Task/For (e.g., int& or int) */ \
        using ArgPassedType = std::tuple_element_t<I, std::tuple<ArgsTypes...>>; \
        /* Type of the corresponding parameter in the Callable's signature (e.g., int& or int) */ \
        using CallableParamType = typename function_traits<CallableType>::template arg_type<I>; \
\
        /* Condition 1: If the passed argument is an lvalue reference */ \
        if constexpr (std::is_lvalue_reference_v<ArgPassedType>) { \
            /* Condition 2: And the callable's parameter is ALSO an lvalue reference */ \
            if constexpr (std::is_lvalue_reference_v<CallableParamType>) { \
                /* Then, we MUST ensure the passed argument was wrapped in std::ref(). */ \
                return is_reference_wrapper_v<std::remove_reference_t<ArgPassedType>>; \
            } else { \
                /* Callable expects by value. Passing an lvalue reference is fine (will be copied). */ \
                return true; \
            } \
        } else { \
            /* Passed an rvalue, copy, or pointer. This is fine. */ \
            return true; \
        } \
    }; \
\
    /* Apply the check to all arguments using an index sequence and fold expression */ \
    static_assert( \
        [&]<size_t... Is>(std::index_sequence<Is...>){ \
            return (... && check_arg_compliance_lambda.template operator()<Is>()); \
        }(std::make_index_sequence<sizeof...(ArgsTypes)>()), \
        "\n\n\t\tError: Lvalue reference arguments intended for reference parameters in the callable " \
        "must be explicitly wrapped in std::ref(). For example: P.Task(my_func, std::ref(my_variable)).\n\n" \
    );