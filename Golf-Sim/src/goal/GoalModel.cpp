#include "GoalModel.h"

#include <glad/glad.h>
#include <terrain/Terrain.h>
#include <terrain/TerrainModel.h>

#include <algorithm>
#include <iostream>
#include <memory>

GoalModel::GoalModel() : numVertices(0) {}

// finds the straight down projection of p onto the plane formed by a, b, c
float projectToPlane(glm::vec2 p, glm::vec3 a, glm::vec3 b, glm::vec3 c) {
  glm::vec3 A = b - a;
  glm::vec3 B = c - a;
  glm::vec3 C = glm::cross(A, B);
  return (C.x * (a.x - p.x) + C.z * (a.z - p.y) + C.y * a.y) / C.y;
}

void printVec3(glm::vec3 vec) {
  std::cout << vec.x << " : " << vec.y << " : " << vec.z << std::endl;
}

float projectToTerrain(glm::vec2 p, Terrain& terrain) {
  float x = p.x;
  float y = p.y;

  float hSpacing = terrain.getHSpacing();
  float vSpacing = terrain.getVSpacing();

  int col = x / hSpacing;
  int row = y / vSpacing;

  float ax = x - hSpacing * col;
  float ay = y - vSpacing * row;

  bool isTopRight = ay * hSpacing > -vSpacing * ax + vSpacing * hSpacing;
  // std::cout << x << " : " << y << " : " << col << " : " << row << " : " <<
  // isTopRight << std::endl;

  float tl = col * hSpacing;
  float tr = (col + 1) * hSpacing;
  float tb = row * vSpacing;
  float tt = (row + 1) * vSpacing;
  glm::vec3 topLeft = glm::vec3(tl, terrain.getHeight(col, row + 1), tt);
  glm::vec3 topRight = glm::vec3(tr, terrain.getHeight(col + 1, row + 1), tt);
  glm::vec3 botLeft = glm::vec3(tl, terrain.getHeight(col, row), tb);
  glm::vec3 botRight = glm::vec3(tr, terrain.getHeight(col + 1, row), tb);
  float height = isTopRight ? projectToPlane(p, topLeft, topRight, botRight)
                            : projectToPlane(p, topLeft, botRight, botLeft);
  // printVec3(topLeft);
  // printVec3(topRight);
  // printVec3(botLeft);
  // printVec3(botRight);
  // std::cout << height << std::endl;
  return height;
}

bool isInCircle(glm::vec2 p, glm::vec2 c, float r) {
  return glm::length(p - c) < r;
}

glm::vec2 get2D(glm::vec3 a) { return glm::vec2(a.x, a.z); }

// 2d cross product
float ccw(glm::vec2 a, glm::vec2 b, glm::vec2 c) {
  return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
}

// from
// https://cp-algorithms.com/geometry/circle-line-intersection.html#solution
// assumes circle is centered at origin and the line is giving in the form ax +
// by + c = 0
std::vector<glm::vec2> circleLineIntersectionOrigin(float r, float a, float b,
                                                    float c) {
  float EPS = 0.01;
  double x0 = -a * c / (a * a + b * b), y0 = -b * c / (a * a + b * b);
  std::vector<glm::vec2> res;
  if (c * c > r * r * (a * a + b * b) + EPS) {
    // return nothing
  } else if (abs(c * c - r * r * (a * a + b * b)) < EPS) {
    res.push_back(glm::vec2(x0, y0));
  } else {
    double d = r * r - c * c / (a * a + b * b);
    double mult = sqrt(d / (a * a + b * b));
    double ax, ay, bx, by;
    ax = x0 + b * mult;
    bx = x0 - b * mult;
    ay = y0 - a * mult;
    by = y0 + a * mult;
    res.push_back(glm::vec2(ax, ay));
    res.push_back(glm::vec2(bx, by));
  }

  return res;
}

std::vector<glm::vec2> circleLineIntersection(glm::vec2 c, float radius,
                                              glm::vec2 a, glm::vec2 b) {
  glm::vec2 an = a - c;
  glm::vec2 bn = b - c;
  float aa = an.y - bn.y;
  float bb = bn.x - an.x;
  float cc = -aa * an.x - bb * an.y;
  // printf("(%f %f) -> (%f %f) : %fx + %fy + %f = 0\n", an.x, an.y, bn.x, bn.y,
  // aa, bb, cc);
  std::vector<glm::vec2> res = circleLineIntersectionOrigin(radius, aa, bb, cc);
  std::vector<glm::vec2> ret;
  for (glm::vec2& x : res) {
    if (glm::dot(x - an, bn - an) >= 0 && glm::dot(x - bn, an - bn) >= 0) {
      ret.push_back(c + x);
    }
  }

  return ret;
}

std::vector<glm::vec3> addHeights(std::vector<glm::vec2> v, Terrain& terrain) {
  std::vector<glm::vec3> res;
  for (glm::vec2 p : v) {
    res.push_back(glm::vec3(p.x, projectToTerrain(p, terrain), p.y));
  }

  return res;
}

void GoalModel::generateModel(Terrain& terrain, glm::vec2 goalCenter,
                              float radius) {
  freeModel();

  pos = terrain.getPosition() -
        glm::vec3(terrain.getWidth() / 2, 0, terrain.getHeight() / 2);

  float hSpacing = terrain.getHSpacing();
  float vSpacing = terrain.getVSpacing();

  float l = goalCenter.x - radius;
  float r = goalCenter.x + radius;
  float b = goalCenter.y - radius;
  float t = goalCenter.y + radius;

  // generate bounding box
  int rl = static_cast<int>(floorf(l / hSpacing));
  int rr = static_cast<int>(ceilf(r / hSpacing));
  int rb = static_cast<int>(floorf(b / vSpacing));
  int rt = static_cast<int>(ceilf(t / vSpacing));

  float averageHeight = 0;

  // generate points on circle and project onto terrain
  std::vector<GoalModelPoint> points(SECTOR_COUNT);
  for (int i = 0; i < SECTOR_COUNT; i++) {
    float angle = i * SECTOR_STEP;
    float x = radius * cosf(angle) + goalCenter.x;
    float y = radius * sinf(angle) + goalCenter.y;
    points[i].pos = glm::vec2(x, y);
    points[i].height = projectToTerrain(points[i].pos, terrain);
    points[i].row = y / vSpacing;
    points[i].col = x / hSpacing;

    averageHeight += points[i].height;
  }

  averageHeight /= SECTOR_COUNT;

  // add triangles around the hole to model
  for (int row = rb; row < rt; row++) {
    for (int col = rl; col < rr; col++) {
      float tl = col * hSpacing;
      float tr = (col + 1) * hSpacing;
      float tb = row * vSpacing;
      float tt = (row + 1) * vSpacing;
      glm::vec3 topLeft = glm::vec3(tl, terrain.getHeight(col, row + 1), tt);
      glm::vec3 topRight =
          glm::vec3(tr, terrain.getHeight(col + 1, row + 1), tt);
      glm::vec3 botLeft = glm::vec3(tl, terrain.getHeight(col, row), tb);
      glm::vec3 botRight = glm::vec3(tr, terrain.getHeight(col + 1, row), tb);

      glm::vec2 cellCenter = glm::vec2((tl + tr) / 2.0, (tb + tt) / 2.0);

      std::vector<glm::vec3> innerPoints;
      for (GoalModelPoint& point : points) {
        if (point.row == row && point.col == col) {
          innerPoints.push_back(
              glm::vec3(point.pos.x, point.height, point.pos.y));
        }
      }

      std::vector<glm::vec3> cornerPoints;
      if (!isInCircle(glm::vec2(topLeft.x, topLeft.z), goalCenter, radius)) {
        cornerPoints.push_back(topLeft);
      }
      if (!isInCircle(glm::vec2(topRight.x, topRight.z), goalCenter, radius)) {
        cornerPoints.push_back(topRight);
      }
      if (!isInCircle(glm::vec2(botLeft.x, botLeft.z), goalCenter, radius)) {
        cornerPoints.push_back(botLeft);
      }
      if (!isInCircle(glm::vec2(botRight.x, botRight.z), goalCenter, radius)) {
        cornerPoints.push_back(botRight);
      }

      // add triangles for cells completely outside circle
      if (innerPoints.empty() && cornerPoints.size() == 4) {
        addTriangle(topLeft, topRight, botRight,
                    getNormal(topLeft, topRight, botRight));
        addTriangle(topLeft, botRight, botLeft,
                    getNormal(topLeft, botRight, botLeft));
        continue;
      }

      if (innerPoints.empty() || cornerPoints.empty()) {
        continue;
      }

      // sort points by their angle they make with the line from the radius to
      // the center
      glm::vec2 gc = cellCenter - goalCenter;
      auto sortAngle = [goalCenter, cellCenter, gc](const glm::vec3& a,
                                                    const glm::vec3& b) {
        glm::vec2 ga = get2D(a) - goalCenter;
        glm::vec2 gb = get2D(b) - goalCenter;
        float da = ccw(goalCenter, cellCenter, get2D(a)) / glm::length(ga);
        float db = ccw(goalCenter, cellCenter, get2D(b)) / glm::length(gb);
        return da < db;
      };

      // add any points that are the result of the cell intersecting with the
      // circle

      std::vector<glm::vec3> topIntersections =
          addHeights(circleLineIntersection(goalCenter, radius, get2D(topLeft),
                                            get2D(topRight)),
                     terrain);
      std::vector<glm::vec3> rightIntersections =
          addHeights(circleLineIntersection(goalCenter, radius, get2D(topRight),
                                            get2D(botRight)),
                     terrain);
      std::vector<glm::vec3> botIntersections =
          addHeights(circleLineIntersection(goalCenter, radius, get2D(botLeft),
                                            get2D(botRight)),
                     terrain);
      std::vector<glm::vec3> leftIntersections =
          addHeights(circleLineIntersection(goalCenter, radius, get2D(botLeft),
                                            get2D(topLeft)),
                     terrain);

      std::vector<glm::vec3> borderPoints;
      borderPoints.insert(borderPoints.end(), topIntersections.begin(),
                          topIntersections.end());
      borderPoints.insert(borderPoints.end(), rightIntersections.begin(),
                          rightIntersections.end());
      borderPoints.insert(borderPoints.end(), botIntersections.begin(),
                          botIntersections.end());
      borderPoints.insert(borderPoints.end(), leftIntersections.begin(),
                          leftIntersections.end());

      innerPoints.insert(innerPoints.end(), borderPoints.begin(),
                         borderPoints.end());
      std::sort(innerPoints.begin(), innerPoints.end(), sortAngle);
      std::sort(cornerPoints.begin(), cornerPoints.end(), sortAngle);

      // printf("(%d %d): ", row, col);
      // for (glm::vec3& x : innerPoints) {
      //	printf("(%f %f %f), ", x.x - 5, x.y - 5, x.z - 5);
      // }
      // printf("\n");

      // filter out points that are very close together
      auto it = innerPoints.begin();
      while (it + 1 != innerPoints.end()) {
        if (glm::length((*it) - (*(it + 1))) < 0.0001) {
          it = innerPoints.erase(it);
        } else {
          it++;
        }
      }

      glm::vec3 norm = getNormal(topLeft, topRight, botRight);

      if (cornerPoints.size() == 1) {
        glm::vec3 cornerPoint = cornerPoints[0];
        for (int i = 0; i < innerPoints.size() - 1; i++) {
          glm::vec3 a = innerPoints[i];
          glm::vec3 b = innerPoints[i + 1];
          addTriangle(a, b, cornerPoint, norm);
        }
      } else if (cornerPoints.size() == 3) {
        glm::vec3 firstCorner = cornerPoints[0];
        glm::vec3 lastCorner = cornerPoints[2];
        glm::vec3 middleCorner = cornerPoints[1];

        addTriangle(*innerPoints.begin(), middleCorner, firstCorner, norm);
        addTriangle(*innerPoints.rbegin(), lastCorner, middleCorner, norm);
        for (int i = 0; i < innerPoints.size() - 1; i++) {
          addTriangle(innerPoints[i], innerPoints[i + 1], middleCorner, norm);
        }
      } else if (cornerPoints.size() == 2) {
        glm::vec3 firstCorner = cornerPoints[0];
        glm::vec3 lastCorner = cornerPoints[1];
        int numConnectPerSide = (innerPoints.size() - 1) / 2;
        for (int i = 0; i < numConnectPerSide; i++) {
          glm::vec3 frontCurrPoint = innerPoints[i];
          glm::vec3 frontNextPoint = innerPoints[i + 1];
          addTriangle(frontCurrPoint, frontNextPoint, firstCorner, norm);

          glm::vec3 backCurrPoint = innerPoints[innerPoints.size() - 1 - i];
          glm::vec3 backNextPoint = innerPoints[innerPoints.size() - 1 - i - 1];
          addTriangle(backCurrPoint, backNextPoint, lastCorner, norm);
        }

        if (innerPoints.size() % 2 == 0) {
          glm::vec3 mid = (firstCorner + lastCorner) * 0.5f;
          addTriangle(innerPoints[numConnectPerSide], firstCorner, mid, norm);
          addTriangle(innerPoints[numConnectPerSide],
                      innerPoints[numConnectPerSide], mid, norm);
          addTriangle(innerPoints[numConnectPerSide + 1], lastCorner, mid,
                      norm);
        } else {
          addTriangle(innerPoints[numConnectPerSide], firstCorner, lastCorner,
                      norm);
        }
      } else {
        assert(false);
      }

      // printf("Patching holes for %d, %d\n", col, row);
      std::vector<GoalModelPoint> pointsCopy(points);
      // patch up holes by adding triangles between points on cell borders and
      // their nearest neighbors
      for (glm::vec3& x : borderPoints) {
        auto sortDist = [x](const GoalModelPoint& a, const GoalModelPoint& b) {
          return glm::length(x - glm::vec3(a.pos.x, a.height, a.pos.y)) <
                 glm::length(x - glm::vec3(b.pos.x, b.height, b.pos.y));
        };

        std::sort(pointsCopy.begin(), pointsCopy.end(), sortDist);

        glm::vec3 a = glm::vec3(pointsCopy[0].pos.x, pointsCopy[0].height,
                                pointsCopy[0].pos.y);
        glm::vec3 b = glm::vec3(pointsCopy[1].pos.x, pointsCopy[1].height,
                                pointsCopy[1].pos.y);

        addTriangle(x, a, b, norm);
      }
    }
  }

  // add walls of the goal
  this->bottomHeight = averageHeight - GOAL_HEIGHT + terrain.getPosition().y;

  for (int i = 0; i < SECTOR_COUNT; i++) {
    GoalModelPoint curr = points[i];
    GoalModelPoint next = points[(i + 1) % SECTOR_COUNT];

    glm::vec3 currTop = glm::vec3(curr.pos.x, curr.height, curr.pos.y);
    glm::vec3 currBottom = glm::vec3(
        curr.pos.x, bottomHeight - terrain.getPosition().y, curr.pos.y);

    glm::vec3 nextTop = glm::vec3(next.pos.x, next.height, next.pos.y);
    glm::vec3 nextBottom = glm::vec3(
        next.pos.x, bottomHeight - terrain.getPosition().y, next.pos.y);

    addTriangle(currTop, currBottom, nextTop,
                getNormal(currTop, currBottom, nextTop));
    addTriangle(nextTop, currBottom, nextBottom,
                getNormal(currTop, currBottom, nextTop));
  }

  // add bottom of the goal
  glm::vec3 centerBottom = glm::vec3(
      goalCenter.x, bottomHeight - terrain.getPosition().y, goalCenter.y);
  for (int i = 0; i < SECTOR_COUNT; i++) {
    GoalModelPoint curr = points[i];
    GoalModelPoint next = points[(i + 1) % SECTOR_COUNT];

    glm::vec3 currBottom = glm::vec3(
        curr.pos.x, bottomHeight - terrain.getPosition().y, curr.pos.y);
    glm::vec3 nextBottom = glm::vec3(
        next.pos.x, bottomHeight - terrain.getPosition().y, next.pos.y);

    addTriangle(currBottom, centerBottom, nextBottom,
                getNormal(currBottom, centerBottom, nextBottom));
  }

  vertexArray = std::make_unique<opengl::VertexArray>();
  vertexArray->bind();

  vertexBuffer = std::make_unique<opengl::VertexBuffer>(
      fullVertexData.size() * sizeof(float), fullVertexData.data(),
      6 * sizeof(float), GL_STATIC_DRAW);
  vertexBuffer->setVertexAttribute(0, 3, GL_FLOAT, 0);
  vertexBuffer->setVertexAttribute(1, 3, GL_FLOAT, 3 * sizeof(float));

  vertexArray->unbind();
}

void GoalModel::addVertex(glm::vec3 a, glm::vec3 norm) {
  fullVertexData.push_back(a.x);
  fullVertexData.push_back(a.y);
  fullVertexData.push_back(a.z);

  fullVertexData.push_back(norm.x);
  fullVertexData.push_back(norm.y);
  fullVertexData.push_back(norm.z);

  int ix = -1;
  for (int i = 0; i < vertices.size(); i += 3) {
    glm::vec3 vertex = glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
    if (glm::length(vertex - a) < 0.00001) {
      ix = i / 3;
    }
  }

  if (ix == -1) {
    vertices.push_back(a.x);
    vertices.push_back(a.y);
    vertices.push_back(a.z);

    ix = (vertices.size() - 3) / 3;
  }

  indices.push_back(static_cast<unsigned int>(ix));

  numVertices++;
}

glm::vec3 GoalModel::getNormal(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
  glm::vec3 A = b - a;
  glm::vec3 B = c - a;
  glm::vec3 C = glm::normalize(glm::cross(A, B));
  // if (C.y < 0) C *= -1;
  return C;
}

void GoalModel::addTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c,
                            glm::vec3 norm) {
  glm::vec3 triangleNorm = getNormal(a, b, c);
  if (norm.y < 0) {
    norm *= -1;
  }

  // ensure the ordering is consistent
  if (triangleNorm.y < 0) {
    norm = getNormal(a, c, b);
    addVertex(a, norm);
    addVertex(c, norm);
    addVertex(b, norm);
  } else {
    addVertex(a, norm);
    addVertex(b, norm);
    addVertex(c, norm);
  }

  numVertices += 3;
}

void GoalModel::freeModel() {
  if (fullVertexData.empty()) return;

  fullVertexData.clear();
  vertices.clear();
  indices.clear();
  numVertices = 0;

  vertexArray->free();
  vertexBuffer->free();
}
