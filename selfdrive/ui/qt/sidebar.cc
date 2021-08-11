#include "selfdrive/ui/qt/sidebar.h"

#include <QMouseEvent>

#include "selfdrive/common/util.h"
#include "selfdrive/hardware/hw.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/widgets/input.h" // opkr
#include "selfdrive/common/params.h" // opkr

#include <QProcess>
#include <QSoundEffect>

void Sidebar::drawMetric(QPainter &p, const QString &label, const QString &val, QColor c, int y) {
  const QRect rect = {30, y, 240, val.isEmpty() ? (label.contains("\n") ? 124 : 100) : 148};

  p.setPen(Qt::NoPen);
  p.setBrush(QBrush(c));
  p.setClipRect(rect.x() + 6, rect.y(), 18, rect.height(), Qt::ClipOperation::ReplaceClip);
  p.drawRoundedRect(QRect(rect.x() + 6, rect.y() + 6, 100, rect.height() - 12), 10, 10);
  p.setClipping(false);

  QPen pen = QPen(QColor(0xff, 0xff, 0xff, 0x55));
  pen.setWidth(2);
  p.setPen(pen);
  p.setBrush(Qt::NoBrush);
  p.drawRoundedRect(rect, 20, 20);

  p.setPen(QColor(0xff, 0xff, 0xff));
  if (val.isEmpty()) {
    configFont(p, "Open Sans", 35, "Bold");
    const QRect r = QRect(rect.x() + 30, rect.y(), rect.width() - 40, rect.height());
    p.drawText(r, Qt::AlignCenter, label);
  } else {
    configFont(p, "Open Sans", 58, "Bold");
    p.drawText(rect.x() + 50, rect.y() + 71, val);
    configFont(p, "Open Sans", 35, "Regular");
    p.drawText(rect.x() + 50, rect.y() + 50 + 77, label);
  }
}

Sidebar::Sidebar(QWidget *parent) : QFrame(parent) {
  home_img = QImage("../assets/images/button_home.png").scaled(180, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  settings_img = QImage("../assets/images/button_settings.png").scaled(settings_btn.width(), settings_btn.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  connect(this, &Sidebar::valueChanged, [=] { update(); });

  setAttribute(Qt::WA_OpaquePaintEvent);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  setFixedWidth(300);
}

void Sidebar::mousePressEvent(QMouseEvent *event) {
  if (settings_btn.contains(event->pos())) {
    QUIState::ui_state.scene.setbtn_count = QUIState::ui_state.scene.setbtn_count + 1;
    if (QUIState::ui_state.scene.setbtn_count > 1) {
      QUIState::ui_state.scene.setbtn_count = 0;
      emit openSettings();
    }
    return;
  }
  // OPKR 
  if (home_btn.contains(event->pos())) {
      QUIState::ui_state.scene.homebtn_count = QUIState::ui_state.scene.homebtn_count + 1;
    if (QUIState::ui_state.scene.homebtn_count > 2) {
      QUIState::ui_state.scene.homebtn_count = 0;
      bool apks_enable = Params().getBool("OpkrApksEnable");
      if (apks_enable) {
        QProcess::execute("/data/openpilot/run_mixplorer.sh");
      } else {
        if (ConfirmationDialog::alert("믹스플로러를 실행하기 위해서는 사용자설정에서 Apks 사용을 활성화해야 합니다(활성화 후 재부팅 필요)", this)) {}
      }
    }
    return;
  }
  // OPKR map overlay
  if (overlay_btn.contains(event->pos()) && QUIState::ui_state.scene.started) {
    QSoundEffect effect;
    effect.setSource(QUrl::fromLocalFile("/data/openpilot/selfdrive/assets/sounds/warning_1.wav"));
    //effect.setLoopCount(1);
    //effect.setLoopCount(QSoundEffect::Infinite);
    //effect.setVolume(0.1);
    float volume = 0.5f;
    if (QUIState::ui_state.scene.scr.nVolumeBoost < 0) {
      volume = 0.0f;
    } else if (QUIState::ui_state.scene.scr.nVolumeBoost > 1) {
      volume = QUIState::ui_state.scene.scr.nVolumeBoost * 0.01;
    }
    effect.setVolume(volume);
    effect.play();
    QProcess::execute("am start --activity-task-on-home com.opkr.maphack/com.opkr.maphack.MainActivity");
    QUIState::ui_state.scene.map_on_top = false;
    QUIState::ui_state.scene.map_on_overlay = !QUIState::ui_state.scene.map_on_overlay;
  }
}

void Sidebar::updateState(const UIState &s) {
  auto &sm = *(s.sm);

  auto deviceState = sm["deviceState"].getDeviceState();
  setProperty("netType", network_type[deviceState.getNetworkType()]);
  int strength = (int)deviceState.getNetworkStrength();
  setProperty("netStrength", strength > 0 ? strength + 1 : 0);

  auto last_ping = deviceState.getLastAthenaPingTime();
  if (last_ping == 0) {
    setProperty("connectStr", "오프라인");
    setProperty("connectStatus", warning_color);
  } else {
    bool online = nanos_since_boot() - last_ping < 80e9;
    setProperty("connectStr",  online ? "온라인" : "오류");
    setProperty("connectStatus", online ? good_color : danger_color);
  }

  QColor tempStatus = danger_color;
  auto ts = deviceState.getThermalStatus();
  if (ts == cereal::DeviceState::ThermalStatus::GREEN) {
    tempStatus = good_color;
  } else if (ts == cereal::DeviceState::ThermalStatus::YELLOW) {
    tempStatus = warning_color;
  }
  setProperty("tempStatus", tempStatus);
  setProperty("tempVal", (int)deviceState.getAmbientTempC());

  QString pandaStr = "차량\n연결됨";
  QColor pandaStatus = good_color;
  if (s.scene.pandaType == cereal::PandaState::PandaType::UNKNOWN) {
    pandaStatus = danger_color;
    pandaStr = "차량\n연결안됨";
  } else if (s.scene.started && !sm["liveLocationKalman"].getLiveLocationKalman().getGpsOK() && s.scene.gpsAccuracyUblox != 0.00) {
    pandaStatus = warning_color;
    pandaStr = "차량연결됨\nGPS검색중";
  } else if (s.scene.satelliteCount > 0) {
  	pandaStr = QString("차량연결됨\nSAT : %1").arg(s.scene.satelliteCount);
  }
  setProperty("pandaStr", pandaStr);
  setProperty("pandaStatus", pandaStatus);

  // opkr
  QString iPAddress = "N/A";
  QString sSID = "---";
  if (network_type[deviceState.getNetworkType()] != "--") {
    std::string m_strip = s.scene.deviceState.getWifiIpAddress();
    std::string m_strssid = s.scene.deviceState.getWifiSSID();
    iPAddress = QString::fromUtf8(m_strip.c_str());
    sSID = QString::fromUtf8(m_strssid.c_str());
  }
  setProperty("iPAddress", iPAddress);
  setProperty("sSID", sSID);

  if (s.sm->updated("deviceState") || s.sm->updated("pandaState")) {
    // atom
    m_battery_img = s.scene.deviceState.getBatteryStatus() == "Charging" ? 1 : 0;
    m_batteryPercent = s.scene.deviceState.getBatteryPercent();
    repaint();
  }
}

void Sidebar::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.setPen(Qt::NoPen);
  p.setRenderHint(QPainter::Antialiasing);

  p.fillRect(rect(), QColor(0, 0, 0));
  // static imgs
  p.setOpacity(0.65);
  p.drawImage(settings_btn.x(), settings_btn.y(), settings_img);
  p.setOpacity(1.0);
  p.drawImage(60, 1080 - 180 - 40, home_img);

  // network
  int x = 58;
  const QColor gray(0x54, 0x54, 0x54);
  for (int i = 0; i < 5; ++i) {
    p.setBrush(i < net_strength ? Qt::white : gray);
    p.drawEllipse(x, 196, 27, 27);
    x += 37;
  }

  configFont(p, "Open Sans", 35, "Regular");
  p.setPen(QColor(0xff, 0xff, 0xff));
  const QRect r = QRect(50, 243, 100, 50);
  p.drawText(r, Qt::AlignHCenter, net_type);

  // metrics
  drawMetric(p, "시스템온도", QString("%1°C").arg(temp_val), temp_status, 378);
  drawMetric(p, panda_str, "", panda_status, 558);
  drawMetric(p, "네트워크\n" + connect_str, "", connect_status, 716);

  // atom - ip
  if( m_batteryPercent <= 1) return;
  const QRect r2 = QRect(35, 295, 230, 50);
  configFont(p, "Open Sans", 28, "Bold");
  p.setPen(Qt::yellow);
  p.drawText(r2, Qt::AlignHCenter, wifi_IPAddress);

  // opkr - ssid
  const QRect r3 = QRect(35, 335, 230, 45);
  configFont(p, "Open Sans", 25, "Bold");
  p.setPen(Qt::white);
  p.drawText(r3, Qt::AlignHCenter, wifi_SSID);

  // atom - battery
  QRect  rect(160, 247, 76, 36);
  QRect  bq(rect.left() + 6, rect.top() + 5, int((rect.width() - 19) * m_batteryPercent * 0.01), rect.height() - 11 );
  QBrush bgBrush("#149948");
  p.fillRect(bq, bgBrush);  
  p.drawImage(rect, battery_imgs[m_battery_img]);

  p.setPen(Qt::white);
  configFont(p, "Open Sans", 25, "Regular");

  char temp_value_str1[32];
  snprintf(temp_value_str1, sizeof(temp_value_str1), "%d%%", m_batteryPercent );
  p.drawText(rect, Qt::AlignCenter, temp_value_str1);
}
