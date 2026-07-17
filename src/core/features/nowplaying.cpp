#include "../../includes.hpp"
#include "features.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <dbus/dbus.h>
#include <string>

// Cached media state
static std::string cachedTitle = "";
static std::string cachedArtist = "";
static std::string cachedStatus = "Stopped"; // Playing, Paused, Stopped
static float lastQueryTime = 0.0f;
static constexpr float QUERY_INTERVAL = 2.0f; // seconds between D-Bus queries

static DBusConnection *dbusConn = nullptr;
static bool dbusInitFailed = false;

// Vinyl animation state
static float vinylAngle = 0.0f;

static bool initDbus() {
  if (dbusConn)
    return true;
  if (dbusInitFailed)
    return false;

  DBusError err;
  dbus_error_init(&err);

  dbusConn = dbus_bus_get(DBUS_BUS_SESSION, &err);
  if (dbus_error_is_set(&err)) {
    dbus_error_free(&err);
    dbusInitFailed = true;
    return false;
  }

  if (!dbusConn) {
    dbusInitFailed = true;
    return false;
  }

  return true;
}

// Helper: extract a string from a D-Bus variant
static std::string extractStringVariant(DBusMessageIter *variantIter) {
  int type = dbus_message_iter_get_arg_type(variantIter);
  if (type == DBUS_TYPE_STRING) {
    const char *val = nullptr;
    dbus_message_iter_get_basic(variantIter, &val);
    return val ? val : "";
  }
  return "";
}

// Helper: extract artist from Metadata variant (array of strings at key
// "xesam:artist")
static std::string extractArtistFromArray(DBusMessageIter *arrayIter) {
  std::string result;
  while (dbus_message_iter_get_arg_type(arrayIter) != DBUS_TYPE_INVALID) {
    if (dbus_message_iter_get_arg_type(arrayIter) == DBUS_TYPE_STRING) {
      const char *val = nullptr;
      dbus_message_iter_get_basic(arrayIter, &val);
      if (val) {
        if (!result.empty())
          result += ", ";
        result += val;
      }
    }
    dbus_message_iter_next(arrayIter);
  }
  return result;
}

// Query MPRIS2 metadata from a specific bus name
static bool queryPlayer(const char *busName) {
  // Get Metadata property
  DBusMessage *msg =
      dbus_message_new_method_call(busName, "/org/mpris/MediaPlayer2",
                                   "org.freedesktop.DBus.Properties", "Get");
  if (!msg)
    return false;

  const char *iface = "org.mpris.MediaPlayer2.Player";
  const char *prop = "Metadata";
  dbus_message_append_args(msg, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING,
                           &prop, DBUS_TYPE_INVALID);

  DBusError err;
  dbus_error_init(&err);
  DBusMessage *reply =
      dbus_connection_send_with_reply_and_block(dbusConn, msg, 200, &err);
  dbus_message_unref(msg);

  if (dbus_error_is_set(&err)) {
    dbus_error_free(&err);
    return false;
  }
  if (!reply)
    return false;

  // Parse the Metadata variant (it's a{sv})
  DBusMessageIter replyIter;
  dbus_message_iter_init(reply, &replyIter);

  if (dbus_message_iter_get_arg_type(&replyIter) != DBUS_TYPE_VARIANT) {
    dbus_message_unref(reply);
    return false;
  }

  DBusMessageIter variantIter;
  dbus_message_iter_recurse(&replyIter, &variantIter);

  if (dbus_message_iter_get_arg_type(&variantIter) != DBUS_TYPE_ARRAY) {
    dbus_message_unref(reply);
    return false;
  }

  DBusMessageIter dictIter;
  dbus_message_iter_recurse(&variantIter, &dictIter);

  std::string title, artist;

  while (dbus_message_iter_get_arg_type(&dictIter) == DBUS_TYPE_DICT_ENTRY) {
    DBusMessageIter entryIter;
    dbus_message_iter_recurse(&dictIter, &entryIter);

    const char *key = nullptr;
    dbus_message_iter_get_basic(&entryIter, &key);
    dbus_message_iter_next(&entryIter);

    // entryIter now points at the variant value
    if (dbus_message_iter_get_arg_type(&entryIter) == DBUS_TYPE_VARIANT) {
      DBusMessageIter valIter;
      dbus_message_iter_recurse(&entryIter, &valIter);

      if (key && strcmp(key, "xesam:title") == 0) {
        title = extractStringVariant(&valIter);
      } else if (key && strcmp(key, "xesam:artist") == 0) {
        // Artist is an array of strings
        if (dbus_message_iter_get_arg_type(&valIter) == DBUS_TYPE_ARRAY) {
          DBusMessageIter artistArrayIter;
          dbus_message_iter_recurse(&valIter, &artistArrayIter);
          artist = extractArtistFromArray(&artistArrayIter);
        }
      }
    }

    dbus_message_iter_next(&dictIter);
  }

  dbus_message_unref(reply);

  if (title.empty())
    return false;

  cachedTitle = title;
  cachedArtist = artist;

  // Now get PlaybackStatus
  msg = dbus_message_new_method_call(busName, "/org/mpris/MediaPlayer2",
                                     "org.freedesktop.DBus.Properties", "Get");
  if (!msg)
    return true; // We got title at least

  prop = "PlaybackStatus";
  dbus_message_append_args(msg, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING,
                           &prop, DBUS_TYPE_INVALID);

  dbus_error_init(&err);
  reply = dbus_connection_send_with_reply_and_block(dbusConn, msg, 200, &err);
  dbus_message_unref(msg);

  if (!dbus_error_is_set(&err) && reply) {
    DBusMessageIter statusReplyIter;
    dbus_message_iter_init(reply, &statusReplyIter);
    if (dbus_message_iter_get_arg_type(&statusReplyIter) == DBUS_TYPE_VARIANT) {
      DBusMessageIter statusVariant;
      dbus_message_iter_recurse(&statusReplyIter, &statusVariant);
      cachedStatus = extractStringVariant(&statusVariant);
    }
    dbus_message_unref(reply);
  } else {
    if (dbus_error_is_set(&err))
      dbus_error_free(&err);
  }

  return true;
}

static void queryMediaInfo() {
  float currentTime = ImGui::GetTime();
  if (currentTime - lastQueryTime < QUERY_INTERVAL)
    return;
  lastQueryTime = currentTime;

  if (!initDbus())
    return;

  // List all bus names and find MPRIS2 players
  DBusMessage *msg = dbus_message_new_method_call(
      "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",
      "ListNames");
  if (!msg)
    return;

  DBusError err;
  dbus_error_init(&err);
  DBusMessage *reply =
      dbus_connection_send_with_reply_and_block(dbusConn, msg, 300, &err);
  dbus_message_unref(msg);

  if (dbus_error_is_set(&err)) {
    dbus_error_free(&err);
    return;
  }
  if (!reply)
    return;

  DBusMessageIter replyIter;
  dbus_message_iter_init(reply, &replyIter);

  if (dbus_message_iter_get_arg_type(&replyIter) != DBUS_TYPE_ARRAY) {
    dbus_message_unref(reply);
    return;
  }

  DBusMessageIter arrayIter;
  dbus_message_iter_recurse(&replyIter, &arrayIter);

  bool found = false;
  while (dbus_message_iter_get_arg_type(&arrayIter) == DBUS_TYPE_STRING) {
    const char *name = nullptr;
    dbus_message_iter_get_basic(&arrayIter, &name);

    if (name && strstr(name, "org.mpris.MediaPlayer2.") == name) {
      if (queryPlayer(name)) {
        found = true;
        break; // Use first active player found
      }
    }
    dbus_message_iter_next(&arrayIter);
  }

  dbus_message_unref(reply);

  if (!found) {
    cachedTitle = "";
    cachedArtist = "";
    cachedStatus = "Stopped";
  }
}

// Truncate a string to fit within maxWidth pixels
static std::string truncateText(const char *text, float maxWidth) {
  ImVec2 size = ImGui::CalcTextSize(text);
  if (size.x <= maxWidth)
    return text;

  std::string truncated = text;
  while (!truncated.empty()) {
    truncated.pop_back();
    std::string test = truncated + "...";
    if (ImGui::CalcTextSize(test.c_str()).x <= maxWidth) {
      return test;
    }
  }
  return "...";
}

void Features::NowPlaying::draw() {
  if (!CONFIGBOOL("Misc>Misc>Misc>Now Playing"))
    return;

  queryMediaInfo();

  if (cachedTitle.empty())
    return;

  bool isPlaying = (cachedStatus == "Playing");
  bool isPaused = (cachedStatus == "Paused");

  // Spin the vinyl when playing
  if (isPlaying) {
    vinylAngle += ImGui::GetIO().DeltaTime * 1.8f; // radians per second
    if (vinylAngle > 6.28318f)
      vinylAngle -= 6.28318f;
  }

  ImDrawList *dl = Globals::drawList;
  if (!dl)
    return;

  // Layout constants
  float discRadius = 22.0f;
  float panelHeight = 52.0f;
  float textMaxWidth = 180.0f;
  float panelPadX = 12.0f;
  float panelPadY = 6.0f;
  float gap = 8.0f;

  // Truncate text
  std::string titleDisp = truncateText(cachedTitle.c_str(), textMaxWidth);
  std::string artistDisp =
      cachedArtist.empty() ? ""
                           : truncateText(cachedArtist.c_str(), textMaxWidth);

  ImVec2 titleSize = ImGui::CalcTextSize(titleDisp.c_str());
  ImVec2 artistSize = artistDisp.empty()
                          ? ImVec2(0, 0)
                          : ImGui::CalcTextSize(artistDisp.c_str());
  float textBlockWidth = std::max(titleSize.x, artistSize.x);

  float totalWidth = discRadius * 2 + gap + textBlockWidth + panelPadX * 2;
  float panelX = (Globals::screenSizeX - totalWidth) * 0.5f;
  float panelY = panelPadY;

  // Panel background (glassmorphism)
  ImVec2 panelMin(panelX, panelY);
  ImVec2 panelMax(panelX + totalWidth, panelY + panelHeight);
  float rounding = 12.0f;

  dl->AddRectFilled(panelMin, panelMax, ImColor(15, 15, 20, 180), rounding);
  dl->AddRect(panelMin, panelMax, ImColor(60, 60, 80, 120), rounding, 0, 1.0f);

  // Vinyl disc center
  float discCx = panelX + panelPadX + discRadius;
  float discCy = panelY + panelHeight * 0.5f;
  ImVec2 discCenter(discCx, discCy);

  // Draw vinyl record
  // Outer disc (dark)
  dl->AddCircleFilled(discCenter, discRadius, ImColor(25, 25, 30, 255), 48);

  // Grooves — concentric rings with slight rotation offset for life
  for (int i = 0; i < 5; i++) {
    float r = discRadius * (0.35f + i * 0.12f);
    float grooveOffset = vinylAngle * (1.0f + i * 0.3f);

    // Draw groove as thin circles with slight alpha variation
    int alpha = 35 + (int)(15.0f * sinf(grooveOffset));
    dl->AddCircle(discCenter, r, ImColor(120, 120, 140, alpha), 48, 0.8f);
  }

  // Outer rim highlight
  dl->AddCircle(discCenter, discRadius, ImColor(50, 50, 60, 200), 48, 1.5f);

  // Center label (purple, matching cheat theme)
  float labelRadius = discRadius * 0.28f;
  dl->AddCircleFilled(discCenter, labelRadius, ImColor(25, 1, 145, 255), 32);
  dl->AddCircle(discCenter, labelRadius, ImColor(60, 20, 180, 255), 32, 1.0f);

  // Center hole
  dl->AddCircleFilled(discCenter, 1.5f, ImColor(10, 10, 15, 255), 16);

  // Spinning notch (visual indicator that it's spinning)
  if (isPlaying || isPaused) {
    float notchR = discRadius * 0.65f;
    float nx = discCx + cosf(vinylAngle) * notchR;
    float ny = discCy + sinf(vinylAngle) * notchR;
    dl->AddCircleFilled(ImVec2(nx, ny), 1.2f,
                        ImColor(180, 180, 200, isPlaying ? 120 : 40), 8);
  }

  // Text position
  float textX = discCx + discRadius + gap;
  float textBlockHeight =
      titleSize.y + (artistDisp.empty() ? 0 : artistSize.y + 2.0f);
  float textStartY = discCy - textBlockHeight * 0.5f;

  // Title (white)
  dl->AddText(ImVec2(textX + 1, textStartY + 1), ImColor(0, 0, 0, 180),
              titleDisp.c_str());
  dl->AddText(ImVec2(textX, textStartY), ImColor(255, 255, 255, 240),
              titleDisp.c_str());

  // Artist (grey)
  if (!artistDisp.empty()) {
    float artistY = textStartY + titleSize.y + 2.0f;
    dl->AddText(ImVec2(textX + 1, artistY + 1), ImColor(0, 0, 0, 140),
                artistDisp.c_str());
    dl->AddText(ImVec2(textX, artistY), ImColor(160, 160, 175, 220),
                artistDisp.c_str());
  }

  // Playback status indicator (small dot)
  float dotX = panelMax.x - panelPadX - 4.0f;
  float dotY = panelY + panelHeight * 0.5f;
  ImColor dotColor = isPlaying  ? ImColor(0, 220, 80, 255)
                     : isPaused ? ImColor(220, 180, 0, 255)
                                : ImColor(120, 120, 120, 255);
  dl->AddCircleFilled(ImVec2(dotX, dotY), 3.0f, dotColor, 16);
}

void Features::NowPlaying::shutdown() {
  if (dbusConn) {
    dbus_connection_unref(dbusConn);
    dbusConn = nullptr;
  }
}
