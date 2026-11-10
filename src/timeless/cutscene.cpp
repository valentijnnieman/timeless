#include "timeless/cutscene.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

float Cutscene::duration() const {
  if (duration_override > 0.0f)
    return duration_override;
  float d = 0.0f;
  auto last = [&](const std::vector<Vec3Key> &ks) {
    if (!ks.empty())
      d = std::max(d, ks.back().t);
  };
  last(camera.position);
  last(camera.lookAt);
  for (const auto &tr : transforms) {
    last(tr.position);
    last(tr.rotationEuler);
    last(tr.scale);
  }
  for (const auto &a : anims)
    d = std::max(d, a.t);
  for (const auto &s : sounds)
    d = std::max(d, s.t);
  for (const auto &dl : dialogue)
    d = std::max(d, dl.t + dl.duration);
  return d;
}

std::string Cutscene::dialogue_at(float t) const {
  std::string s;
  for (const auto &dl : dialogue)
    if (t >= dl.t && t < dl.t + dl.duration)
      s = dl.text;
  return s;
}

glm::vec3 sample(const std::vector<Vec3Key> &ks, float t) {
  if (ks.empty())
    return glm::vec3(0.0f);
  if (t <= ks.front().t)
    return ks.front().v;
  if (t >= ks.back().t)
    return ks.back().v;
  size_t i = 1;
  while (i < ks.size() && ks[i].t < t)
    ++i;
  const Vec3Key &a = ks[i - 1];
  const Vec3Key &b = ks[i];
  float span = b.t - a.t;
  float u = span > 1e-6f ? (t - a.t) / span : 0.0f;
  return glm::mix(a.v, b.v, ease_apply(b.ease, u));
}

glm::quat look_rotation(const glm::vec3 &pos, const glm::vec3 &target,
                        const glm::vec3 &baseForward, const glm::vec3 &baseUp,
                        const glm::vec3 &worldUp) {
  glm::vec3 f = target - pos;
  if (glm::length(f) < 1e-6f)
    return glm::quat(1, 0, 0, 0);
  f = glm::normalize(f);

  glm::vec3 wUp = worldUp;
  if (std::abs(glm::dot(f, wUp)) > 0.999f) // looking straight up/down: pick a
    wUp = glm::vec3(0, 1, 0);              // different reference to avoid a NaN
  glm::vec3 r = glm::normalize(glm::cross(f, wUp));
  glm::vec3 u = glm::cross(r, f);

  // Orthonormalised camera-local frame (right, up, forward).
  glm::vec3 lf = glm::normalize(baseForward);
  glm::vec3 lr = glm::normalize(glm::cross(lf, glm::normalize(baseUp)));
  glm::vec3 lu = glm::cross(lr, lf);

  glm::mat3 L(lr, lu, lf); // columns: maps local basis -> these vectors
  glm::mat3 W(r, u, f);    // columns: desired world basis
  return glm::quat_cast(W * glm::transpose(L));
}

bool save_cutscene(const Cutscene &cs, const std::string &path) {
  std::ofstream out(path, std::ios::trunc);
  if (!out)
    return false;
  auto key = [&](const char *tag, const Vec3Key &k) {
    out << tag << ' ' << k.t << ' ' << k.v.x << ' ' << k.v.y << ' ' << k.v.z
        << ' ' << ease_name(k.ease) << '\n';
  };
  out << "# beach cutscene v1\n";
  out << "loop " << (cs.loop ? 1 : 0) << '\n';
  out << "duration " << cs.duration_override << '\n';
  for (const auto &k : cs.camera.position)
    key("cam_pos", k);
  for (const auto &k : cs.camera.lookAt)
    key("cam_look", k);
  for (const auto &tr : cs.transforms) {
    out << "track " << tr.target << '\n';
    for (const auto &k : tr.position)
      key("pos", k);
    for (const auto &k : tr.rotationEuler)
      key("rot", k);
    for (const auto &k : tr.scale)
      key("scale", k);
  }
  for (const auto &a : cs.anims)
    out << "anim " << a.t << ' ' << a.target << ' ' << a.clip << '\n';
  for (const auto &s : cs.sounds)
    out << "sound " << s.t << ' ' << s.event << '\n';
  for (const auto &d : cs.dialogue)
    out << "dialogue " << d.t << ' ' << d.duration << ' ' << d.text << '\n';
  return true;
}

bool load_cutscene(Cutscene &cs, const std::string &path) {
  std::ifstream in(path);
  if (!in)
    return false;
  cs = Cutscene{};
  int cur = -1; // index into cs.transforms for the active `track`
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#')
      continue;
    std::istringstream ss(line);
    std::string tag;
    ss >> tag;
    auto read_key = [&]() {
      Vec3Key k;
      std::string ease;
      ss >> k.t >> k.v.x >> k.v.y >> k.v.z >> ease;
      k.ease = ease_from_string(ease);
      return k;
    };
    if (tag == "loop") {
      int v = 0;
      ss >> v;
      cs.loop = v != 0;
    } else if (tag == "duration") {
      ss >> cs.duration_override;
    } else if (tag == "cam_pos") {
      cs.camera.position.push_back(read_key());
    } else if (tag == "cam_look") {
      cs.camera.lookAt.push_back(read_key());
    } else if (tag == "track") {
      TransformTrack tr;
      ss >> tr.target;
      cs.transforms.push_back(tr);
      cur = (int)cs.transforms.size() - 1;
    } else if (tag == "pos" || tag == "rot" || tag == "scale") {
      if (cur < 0)
        continue;
      Vec3Key k = read_key();
      if (tag == "pos")
        cs.transforms[cur].position.push_back(k);
      else if (tag == "rot")
        cs.transforms[cur].rotationEuler.push_back(k);
      else
        cs.transforms[cur].scale.push_back(k);
    } else if (tag == "anim") {
      AnimTrigger a;
      ss >> a.t >> a.target;
      std::getline(ss, a.clip);            // clip = rest of line
      if (!a.clip.empty() && a.clip[0] == ' ')
        a.clip.erase(0, a.clip.find_first_not_of(' '));
      cs.anims.push_back(a);
    } else if (tag == "sound") {
      SoundCue s;
      ss >> s.t;
      std::getline(ss, s.event);             // event = rest of line
      if (!s.event.empty() && s.event[0] == ' ')
        s.event.erase(0, s.event.find_first_not_of(' '));
      cs.sounds.push_back(s);
    } else if (tag == "dialogue") {
      DialogueLine d;
      ss >> d.t >> d.duration;
      std::getline(ss, d.text);            // text = rest of line
      if (!d.text.empty() && d.text[0] == ' ')
        d.text.erase(0, d.text.find_first_not_of(' '));
      cs.dialogue.push_back(d);
    }
  }
  return true;
}
