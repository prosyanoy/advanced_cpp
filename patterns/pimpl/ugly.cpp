#include "bad.h"
#include "ugly.h"

struct SplineImpl {
    double* X = nullptr;
    double* Y = nullptr;
    double* Y2 = nullptr;
    size_t n;
    double A;
    double B;
    double* output;

    SplineImpl(const std::vector<double>& x, const std::vector<double>& y, double a, double b)
        : A(a), B(b) {
        n = x.size();
        X = new double[n];
        Y = new double[n];
        Y2 = new double[n];
        for (size_t i = 0; i < n; ++i) {
            X[i] = x[i];
            Y[i] = y[i];
        }
        mySplineSnd(X, Y, n, A, B, Y2);
    }

    double Interpolate(double x) {
        output = new double();
        mySplintCube(X, Y, Y2, n, x, output);
        double temp = *output;
        delete output;
        return temp;
    }

    ~SplineImpl() {
        if (X) {
            delete[] X;
        }
        if (Y) {
            delete[] Y;
        }
        if (Y2) {
            delete[] Y2;
        }
    }
};

Spline::Spline(const std::vector<double>& x, const std::vector<double>& y, double a, double b)
    : impl_(std::make_shared<SplineImpl>(x, y, a, b)) {
}

double Spline::Interpolate(double x) {
    return impl_->Interpolate(x);
}