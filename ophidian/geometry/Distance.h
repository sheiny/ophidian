/*
 * Copyright 2017 Ophidian
   Licensed to the Apache Software Foundation (ASF) under one
   or more contributor license agreements.  See the NOTICE file
   distributed with this work for additional information
   regarding copyright ownership.  The ASF licenses this file
   to you under the Apache License, Version 2.0 (the
   "License"); you may not use this file except in compliance
   with the License.  You may obtain a copy of the License at
   http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing,
   software distributed under the License is distributed on an
   "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
   KIND, either express or implied.  See the License for the
   specific language governing permissions and limitations
   under the License.
 */

#ifndef OPHIDIAN_GEOMETRY_DISTANCE_H
#define OPHIDIAN_GEOMETRY_DISTANCE_H

#include "Models.h"
#include <ophidian/util/Units.h>

namespace ophidian::geometry
{
    /*!
     * \brief Calculates the Manhattan distance between two points
     * \param point1 First point to calculate the distance
     * \param point2 Second point to calculate the distance
     * \return Manhattan distance between point1 and point2
     */
    template <typename UnitType>
    typename std::enable_if<units::traits::is_unit_t<UnitType>::value, UnitType>::type
    ManhattanDistance(const Point<UnitType> & point1, const Point<UnitType> & point2)
    {
        auto distance = units::math::abs(point1.x() - point2.x()) + units::math::abs(point1.y() - point2.y());

        return distance;
    }

    template <typename UnitType>
    typename std::enable_if<std::is_arithmetic<UnitType>::value, UnitType>::type
    ManhattanDistance(const Point<UnitType> & point1, const Point<UnitType> & point2)
    {
        auto distance = std::abs(point1.x() - point2.x()) + std::abs(point1.y() - point2.y());

        return distance;
    }

    /*!
     * \brief Calculates the Euclidean distance between two points
     * \param point1 First point to calculate the distance
     * \param point2 Second point to calculate the distance
     * \return Euclidean distance between point1 and point2
     */
    template <typename T>
    T EuclideanDistance(const Point<T> & point1, const Point<T> & point2)
    {
        T distanceX = (point1.x() - point2.x()) * (point1.x() - point2.x());
        T distanceY = (point1.y() - point2.y()) * (point1.y() - point2.y());
        T distance = std::sqrt(distanceX + distanceY);

        return distance;
    }
}

#endif // OPHIDIAN_GEOMETRY_DISTANCE_H
