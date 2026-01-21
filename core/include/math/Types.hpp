/*
 * Copyright 2026 Evan M.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#pragma once

namespace ClothSDK {

struct Triangle {
    int a, b, c;
    Triangle(int _a, int _b, int _c) : a(_a), b(_b), c(_c) {}
};


struct ClothMaterial {
    double density;
    double structuralCompliance;
    double shearCompliance;
    double bendingCompliance;

    ClothMaterial(double _density, double _structuralCompliance, double _shearCompliance, double _bendingCompliance)
    : density(_density), structuralCompliance(_structuralCompliance), shearCompliance(_shearCompliance), bendingCompliance(_bendingCompliance) {}

    ClothMaterial() : density(0.1), structuralCompliance(1e-6), shearCompliance(1e-6), bendingCompliance(0.01) {}

    inline double getDensity() const { return density; }
    inline double getStructuralCompliance() const { return structuralCompliance; }
    inline double getShearCompliance() const { return shearCompliance; }
    inline double getBendingCompliance() const { return bendingCompliance; }

};

}