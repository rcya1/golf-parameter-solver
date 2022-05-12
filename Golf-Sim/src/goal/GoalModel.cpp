#include "GoalModel.h"

#include <algorithm>

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
  std::cout << x << " : " << y << " : " << col << " : " << row << " : " << isTopRight << std::endl;

  float tl = col * hSpacing;
  float tr = (col + 1) * hSpacing;
  float tb = row * vSpacing;
  float tt = (row + 1) * vSpacing;
  glm::vec3 topLeft = glm::vec3(tl, terrain.getHeight(col, row + 1), tt);
  glm::vec3 topRight = glm::vec3(tr, terrain.getHeight(col + 1, row + 1), tt);
  glm::vec3 botLeft = glm::vec3(tl, terrain.getHeight(col, row), tb);
  glm::vec3 botRight = glm::vec3(tr, terrain.getHeight(col + 1, row), tb);
  float height = isTopRight ? projectToPlane(p, topLeft, topRight, botRight)
                           : projectToPlane(p, topLeft, botLeft, botRight);
  printVec3(topLeft);
  printVec3(topRight);
  printVec3(botLeft);
  printVec3(botRight);
  std::cout << height << std::endl;
  return height;
}

bool isInCircle(glm::vec2 p, glm::vec2 c, float r) {
  return (p - c).length() < r;
}

glm::vec2 get2D(glm::vec3 a) { return glm::vec2(a.x, a.z); }

// 2d cross product
int ccw(glm::vec2 a, glm::vec2 b, glm::vec2 c) {
  return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
}

std::vector<glm::vec2> circleLineIntersection(glm::vec2 c, float radius,
                                              glm::vec2 a, glm::vec2 b) {
  std::vector<glm::vec2> ans;

  glm::vec2 ac = c - a;
  glm::vec2 ab = b - a;

  float projScale = glm::dot(ac, ab) / (glm::dot(ab, ab));
  if (projScale < 0 || projScale > 1) {
    return ans;
  }
  glm::vec2 d = projScale * ab + a;

  glm::vec2 cd = d - c;
  if (cd.length() > radius) {
    return ans;
  }

  float dist = sqrt(radius * radius - cd.length() * cd.length());
  float distScale = dist / ab.length();

  if (projScale - distScale >= 0 && projScale - distScale <= 1) {
    ans.push_back(a + ab * (projScale - distScale));
  }

  if (projScale + distScale >= 0 && projScale + distScale <= 1) {
    ans.push_back(a + ab * (projScale + distScale));
  }

  return ans;
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
      if (!isInCircle(topLeft, goalCenter, radius)) {
        cornerPoints.push_back(topLeft);
      }
      if (!isInCircle(topRight, goalCenter, radius)) {
        cornerPoints.push_back(topRight);
      }
      if (!isInCircle(botLeft, goalCenter, radius)) {
        cornerPoints.push_back(botLeft);
      }
      if (!isInCircle(botRight, goalCenter, radius)) {
        cornerPoints.push_back(botRight);
      }

      // add triangles for cells completely outside circle
      if (innerPoints.empty() && cornerPoints.size() == 4) {
        //addTriangle(topLeft, topRight, botLeft);
        //addTriangle(topRight, botRight, botLeft);
        continue;
      }

      // TODO for debugging: for every cell, print which points are inside / outside
      // for ever point, print the row / column it is in

      continue;

      if (innerPoints.empty() || cornerPoints.empty()) {
        continue;
      }

      // sort points by their angle they make with the line from the radius to
      // the center
      auto sortAngle = [goalCenter, cellCenter](const glm::vec3& a,
                                                const glm::vec3& b) {
        return ccw(goalCenter, cellCenter, get2D(a)) >
               ccw(goalCenter, cellCenter, get2D(b));
      };

      // add any points that are the result of the cell intersecting with the
      // circle
      std::vector<glm::vec3> topIntersections = addHeights(
          circleLineIntersection(goalCenter, radius, topLeft, topRight),
          terrain);
      std::vector<glm::vec3> rightIntersections = addHeights(
          circleLineIntersection(goalCenter, radius, topRight, botRight),
          terrain);
      std::vector<glm::vec3> botIntersections = addHeights(
          circleLineIntersection(goalCenter, radius, botRight, botLeft),
          terrain);
      std::vector<glm::vec3> leftIntersections = addHeights(
          circleLineIntersection(goalCenter, radius, botLeft, topLeft),
          terrain);

      innerPoints.insert(innerPoints.end(), topIntersections.begin(),
                         topIntersections.end());
      innerPoints.insert(innerPoints.end(), rightIntersections.begin(),
                         rightIntersections.end());
      innerPoints.insert(innerPoints.end(), botIntersections.begin(),
                         botIntersections.end());
      innerPoints.insert(innerPoints.end(), leftIntersections.begin(),
                         leftIntersections.end());

      std::sort(innerPoints.begin(), innerPoints.end(), sortAngle);
      std::sort(cornerPoints.begin(), cornerPoints.end(), sortAngle);

      // filter out points that are very close together
      auto it = innerPoints.begin();
      while (it + 1 != innerPoints.end()) {
        if (((*it) - (*(it + 1))).length() < 0.01) {
          it = innerPoints.erase(it);
        } else {
          it++;
        }
      }

      if (cornerPoints.size() == 1) {
        glm::vec3 cornerPoint = cornerPoints[0];
        for (int i = 0; i < innerPoints.size() - 1; i++) {
          glm::vec3 a = innerPoints[i];
          glm::vec3 b = innerPoints[i + 1];
          addTriangle(a, b, cornerPoint);
        }
      } else if (cornerPoints.size() == 3) {
        glm::vec3 firstCorner = cornerPoints[0];
        glm::vec3 lastCorner = cornerPoints[2];
        glm::vec3 middleCorner = cornerPoints[1];

        addTriangle(*innerPoints.begin(), firstCorner, middleCorner);
        addTriangle(*innerPoints.rbegin(), lastCorner, middleCorner);
        for (int i = 0; i < innerPoints.size() - 1; i++) {
          addTriangle(innerPoints[i], innerPoints[i + 1], middleCorner);
        }
      } else if (cornerPoints.size() == 2) {
        glm::vec3 firstCorner = cornerPoints[0];
        glm::vec3 lastCorner = cornerPoints[1];
        int numConnectPerSide = (innerPoints.size() - 1) / 2;
        for (int i = 0; i < numConnectPerSide; i++) {
          glm::vec3 frontCurrPoint = innerPoints[i];
          glm::vec3 frontNextPoint = innerPoints[i + 1];
          addTriangle(frontCurrPoint, frontNextPoint, firstCorner);

          glm::vec3 backCurrPoint = innerPoints[innerPoints.size() - 1 - i];
          glm::vec3 backNextPoint = innerPoints[innerPoints.size() - 1 - i - 1];
          addTriangle(backCurrPoint, backNextPoint, lastCorner);
        }

        if (innerPoints.size() % 2 == 0) {
          glm::vec3 mid = (firstCorner + lastCorner) * 0.5f;
          addTriangle(innerPoints[numConnectPerSide], firstCorner, mid);
          addTriangle(innerPoints[numConnectPerSide],
                      innerPoints[numConnectPerSide], mid);
          addTriangle(innerPoints[numConnectPerSide + 1], lastCorner, mid);
        } else {
          addTriangle(innerPoints[numConnectPerSide], firstCorner, lastCorner);
        }
      } else {
        assert(false);
      }
    }
  }

  // add walls of the goal
  float bottomHeight = averageHeight - GOAL_HEIGHT;

  for (int i = 0; i < SECTOR_COUNT; i++) {
    GoalModelPoint curr = points[i];
    GoalModelPoint next = points[(i + 1) % SECTOR_COUNT];

    glm::vec3 currTop = glm::vec3(curr.pos.x, curr.height, curr.pos.y);
    glm::vec3 currBottom = glm::vec3(curr.pos.x, bottomHeight, curr.pos.y);

    glm::vec3 nextTop = glm::vec3(next.pos.x, next.height, next.pos.y);
    glm::vec3 nextBottom = glm::vec3(next.pos.x, bottomHeight, next.pos.y);

    addTriangle(currTop, nextTop, currBottom);
    addTriangle(nextTop, currBottom, nextBottom);
  }

  // add bottom of the goal
  glm::vec3 centerBottom = glm::vec3(goalCenter.x, bottomHeight, goalCenter.y);
  for (int i = 0; i < SECTOR_COUNT; i++) {
    GoalModelPoint curr = points[i];
    GoalModelPoint next = points[(i + 1) % SECTOR_COUNT];

    glm::vec3 currBottom = glm::vec3(curr.pos.x, bottomHeight, curr.pos.y);
    glm::vec3 nextBottom = glm::vec3(next.pos.x, bottomHeight, next.pos.y);

    addTriangle(currBottom, nextBottom, centerBottom);
  }

  vertexArray = std::make_unique<opengl::VertexArray>();
  vertexArray->bind();

  vertexBuffer = std::make_unique<opengl::VertexBuffer>(
      vertices.size() * sizeof(float), vertices.data(), 6 * sizeof(float),
      GL_STATIC_DRAW);
  vertexBuffer->setVertexAttribute(0, 3, GL_FLOAT, 0);
  vertexBuffer->setVertexAttribute(1, 3, GL_FLOAT, 3 * sizeof(float));

  vertexArray->unbind();
}

void GoalModel::addVertex(glm::vec3 a, glm::vec3 norm) {
  vertices.push_back(a.x);
  vertices.push_back(a.y);
  vertices.push_back(a.z);

  vertices.push_back(norm.x);
  vertices.push_back(norm.y);
  vertices.push_back(norm.z);

  numVertices++;
}

glm::vec3 GoalModel::getNormal(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
  glm::vec3 A = b - a;
  glm::vec3 B = c - a;
  glm::vec3 C = glm::normalize(glm::cross(A, B));
  if (C.y < 0) C *= -1;
  return C;
}

void GoalModel::addTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
  glm::vec3 norm = getNormal(a, b, c);
  addVertex(a, norm);
  addVertex(b, norm);
  addVertex(c, norm);

  numVertices += 3;
}

void GoalModel::freeModel() {
  if (vertices.empty()) return;

  vertices.clear();
  numVertices = 0;

  vertexArray->free();
  vertexBuffer->free();
}
