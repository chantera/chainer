#pragma once

#include <utility>

#include <nonstd/optional.hpp>

#include "chainerx/array.h"
#include "chainerx/dtype.h"
#include "chainerx/macro.h"
#include "chainerx/scalar.h"

namespace chainerx {
namespace type_util_detail {

class ResultTypeResolver {
public:
    template <typename Arg, typename... Args>
    Dtype ResolveArgs(Arg arg, Args... args) {
        // At least single argument is required.
        AddArgsImpl(std::forward<Arg>(arg));
        AddArgsImpl(std::forward<Args>(args)...);
        return Resolve();
    }

    Dtype Resolve() const;

    void AddArg(const Array& arg);

    void AddArg(Scalar arg);

private:
    nonstd::optional<Dtype> array_max_dtype_;
    nonstd::optional<Dtype> scalar_max_dtype_;

    // Returns the minimal dtype which can be safely casted from both dtypes.
    static Dtype PromoteType(Dtype dt1, Dtype dt2);

    void AddArgsImpl() {
        // nop
    }

    template <typename Arg, typename... Args>
    void AddArgsImpl(Arg arg, Args... args) {
        AddArg(std::forward<Arg>(arg));
        AddArgsImpl(std::forward<Args>(args)...);
    }

    static int GetDtypeCategory(Dtype dtype) {
        switch (GetKind(dtype)) {
            case DtypeKind::kFloat:
                return 2;
            default:
                return 1;
        }
    }
};

}  // namespace type_util_detail

inline Dtype ResultType(const Array& arg) { return arg.dtype(); }

inline Dtype ResultType(Scalar arg) { return arg.dtype(); }

template <typename Arg, typename... Args>
Dtype ResultType(Arg arg, Args... args) {
    return type_util_detail::ResultTypeResolver{}.ResolveArgs(std::forward<Arg>(arg), std::forward<Args>(args)...);
}

template <typename Container>
Dtype ResultType(Container args) {
    type_util_detail::ResultTypeResolver resolver{};
    if (args.size() == 0U) {
        throw ChainerxError{"At least one argument is required."};
    }
    for (const Array& arg : args) {
        resolver.AddArg(arg);
    }
    return resolver.Resolve();
}

}  // namespace chainerx
