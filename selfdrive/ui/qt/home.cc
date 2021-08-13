#include "selfdrive/ui/qt/home.h"

#include <QDate>
#include <QTime>
#include <QLocale>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QProcess> // opkr
#include <QSoundEffect> // opkr

#include "selfdrive/common/params.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/widgets/drive_stats.h"
#include "selfdrive/ui/qt/widgets/prime.h"

// HomeWindow: the container for the offroad and onroad UIs

HomeWindow::HomeWindow(QWidget* parent) : QWidget(parent) {
  QHBoxLayout *main_layout = new QHBoxLayout(this);
  main_layout->setMargin(0);
  main_layout->setSpacing(0);

  sidebar = new Sidebar(this);
  main_layout->addWidget(sidebar);
  QObject::connect(this, &HomeWindow::update, sidebar, &Sidebar::updateState);
  QObject::connect(sidebar, &Sidebar::openSettings, this, &HomeWindow::openSettings);

  slayout = new QStackedLayout();
  main_layout->addLayout(slayout);

  home = new OffroadHome();
  slayout->addWidget(home);

  onroad = new OnroadWindow(this);
  slayout->addWidget(onroad);

  QObject::connect(this, &HomeWindow::update, onroad, &OnroadWindow::updateStateSignal);
  QObject::connect(this, &HomeWindow::offroadTransitionSignal, onroad, &OnroadWindow::offroadTransitionSignal);

  driver_view = new DriverViewWindow(this);
  connect(driver_view, &DriverViewWindow::done, [=] {
    showDriverView(false);
  });
  slayout->addWidget(driver_view);
  setAttribute(Qt::WA_NoSystemBackground);
}

void HomeWindow::showSidebar(bool show) {
  sidebar->setVisible(show);
}

void HomeWindow::offroadTransition(bool offroad) {
  sidebar->setVisible(offroad);
  if (offroad) {
    slayout->setCurrentWidget(home);
  } else {
    slayout->setCurrentWidget(onroad);
  }
  emit offroadTransitionSignal(offroad);
}

void HomeWindow::showDriverView(bool show) {
  if (show) {
    emit closeSettings();
    slayout->setCurrentWidget(driver_view);
  } else {
    slayout->setCurrentWidget(home);
  }
  sidebar->setVisible(show == false);
}

void HomeWindow::mousePressEvent(QMouseEvent* e) {
  // OPKR add map
  if (QUIState::ui_state.scene.apks_enabled && QUIState::ui_state.scene.started && map_overlay_btn.ptInRect(e->x(), e->y())) {
    QSoundEffect effect1;
    effect1.setSource(QUrl::fromLocalFile("/data/openpilot/selfdrive/assets/sounds/warning_1.wav"));
    //effect1.setLoopCount(1);
    //effect1.setLoopCount(QSoundEffect::Infinite);
    float volume1 = 0.5;
    if (QUIState::ui_state.scene.scr.nVolumeBoost < 0) {
      volume1 = 0.0;
    } else if (QUIState::ui_state.scene.scr.nVolumeBoost > 1) {
      volume1 = QUIState::ui_state.scene.scr.nVolumeBoost * 0.01;
    }
    effect1.setVolume(volume1);
    effect1.play();
    QProcess::execute("am start --activity-task-on-home com.opkr.maphack/com.opkr.maphack.MainActivity");
    QUIState::ui_state.scene.map_on_top = false;
    QUIState::ui_state.scene.map_on_overlay = true;
    return;
  }
  if (QUIState::ui_state.scene.apks_enabled && QUIState::ui_state.scene.started && !sidebar->isVisible() && !QUIState::ui_state.scene.map_on_top && map_btn.ptInRect(e->x(), e->y())) {
    QSoundEffect effect2;
    effect2.setSource(QUrl::fromLocalFile("/data/openpilot/selfdrive/assets/sounds/warning_1.wav"));
    //effect1.setLoopCount(1);
    //effect1.setLoopCount(QSoundEffect::Infinite);
    float volume2 = 0.5;
    if (QUIState::ui_state.scene.scr.nVolumeBoost < 0) {
      volume2 = 0.0;
    } else if (QUIState::ui_state.scene.scr.nVolumeBoost > 1) {
      volume2 = QUIState::ui_state.scene.scr.nVolumeBoost * 0.01;
    }
    effect2.setVolume(volume2);
    effect2.play();
    QUIState::ui_state.scene.map_is_running = !QUIState::ui_state.scene.map_is_running;
    if (QUIState::ui_state.scene.map_is_running) {
      QProcess::execute("am start com.mnsoft.mappyobn/com.mnsoft.mappy.MainActivity");
      QUIState::ui_state.scene.map_on_top = true;
      QUIState::ui_state.scene.map_is_running = true;
      QUIState::ui_state.scene.map_on_overlay = false;
      Params().putBool("OpkrMapEnable", true);
    } else {
      QProcess::execute("pkill com.mnsoft.mappyobn");
      QUIState::ui_state.scene.map_on_top = false;
      QUIState::ui_state.scene.map_on_overlay = false;
      QUIState::ui_state.scene.map_is_running = false;
      Params().putBool("OpkrMapEnable", false);
    }
    return;
  }
  if (QUIState::ui_state.scene.apks_enabled && QUIState::ui_state.scene.started && QUIState::ui_state.scene.map_is_running && map_return_btn.ptInRect(e->x(), e->y())) {
    QSoundEffect effect3;
    effect3.setSource(QUrl::fromLocalFile("/data/openpilot/selfdrive/assets/sounds/warning_1.wav"));
    //effect1.setLoopCount(1);
    //effect1.setLoopCount(QSoundEffect::Infinite);
    float volume3 = 0.5;
    if (QUIState::ui_state.scene.scr.nVolumeBoost < 0) {
      volume3 = 0.0;
    } else if (QUIState::ui_state.scene.scr.nVolumeBoost > 1) {
      volume3 = QUIState::ui_state.scene.scr.nVolumeBoost * 0.01;
    }
    effect3.setVolume(volume3);
    effect3.play();
    QProcess::execute("am start --activity-task-on-home com.mnsoft.mappyobn/com.mnsoft.mappy.MainActivity");
    QUIState::ui_state.scene.map_on_top = true;
    QUIState::ui_state.scene.map_on_overlay = false;
    return;
  }
  // OPKR REC
  if (QUIState::ui_state.scene.started && !sidebar->isVisible() && !QUIState::ui_state.scene.map_on_top && !QUIState::ui_state.scene.comma_stock_ui && rec_btn.ptInRect(e->x(), e->y())) {
    QUIState::ui_state.scene.recording = !QUIState::ui_state.scene.recording;
    QUIState::ui_state.scene.touched = true;
    return;
  }
  // Laneless mode
  if (QUIState::ui_state.scene.started && !sidebar->isVisible() && !QUIState::ui_state.scene.map_on_top && QUIState::ui_state.scene.end_to_end && !QUIState::ui_state.scene.comma_stock_ui && laneless_btn.ptInRect(e->x(), e->y())) {
    QUIState::ui_state.scene.laneless_mode = QUIState::ui_state.scene.laneless_mode + 1;
    if (QUIState::ui_state.scene.laneless_mode > 2) {
      QUIState::ui_state.scene.laneless_mode = 0;
    }
    if (QUIState::ui_state.scene.laneless_mode == 0) {
      Params().put("LanelessMode", "0", 1);
    } else if (QUIState::ui_state.scene.laneless_mode == 1) {
      Params().put("LanelessMode", "1", 1);
    } else if (QUIState::ui_state.scene.laneless_mode == 2) {
      Params().put("LanelessMode", "2", 1);
    }
    return;
  }
  // Monitoring mode
  if (QUIState::ui_state.scene.started && !sidebar->isVisible() && !QUIState::ui_state.scene.map_on_top && monitoring_btn.ptInRect(e->x(), e->y())) {
    QUIState::ui_state.scene.monitoring_mode = !QUIState::ui_state.scene.monitoring_mode;
    if (QUIState::ui_state.scene.monitoring_mode) {
      Params().putBool("OpkrMonitoringMode", true);
    } else {
      Params().putBool("OpkrMonitoringMode", false);
    }
    return;
  }
  // Stock UI Toggle
  if (QUIState::ui_state.scene.started && !sidebar->isVisible() && !QUIState::ui_state.scene.map_on_top && stockui_btn.ptInRect(e->x(), e->y())) {
    QUIState::ui_state.scene.comma_stock_ui = !QUIState::ui_state.scene.comma_stock_ui;
    if (QUIState::ui_state.scene.comma_stock_ui) {
      Params().putBool("CommaStockUI", true);
    } else {
      Params().putBool("CommaStockUI", false);
    }
    return;
  }
  // opkr live camera offset
  if (QUIState::ui_state.scene.live_tune_panel_enable) {
    if (QUIState::ui_state.scene.started && !sidebar->isVisible() && !QUIState::ui_state.scene.map_on_top && livetunepanel_left_btn.ptInRect(e->x(), e->y())) {
      if (QUIState::ui_state.scene.live_tune_panel_list == 0) {
        QUIState::ui_state.scene.cameraOffset = QUIState::ui_state.scene.cameraOffset - 5;
        if (QUIState::ui_state.scene.cameraOffset <= -1000) QUIState::ui_state.scene.cameraOffset = -1000;
        QString value = QString::number(QUIState::ui_state.scene.cameraOffset);
        Params().put("CameraOffsetAdj", value.toStdString());
        return;
      }
      if (QUIState::ui_state.scene.live_tune_panel_list == 1 && QUIState::ui_state.scene.lateralControlMethod == 0) {
        QUIState::ui_state.scene.pidKp = QUIState::ui_state.scene.pidKp - 1;
        if (QUIState::ui_state.scene.pidKp <= 1) QUIState::ui_state.scene.pidKp = 1;
        QString value = QString::number(QUIState::ui_state.scene.pidKp);
        Params().put("PidKp", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 2 && QUIState::ui_state.scene.lateralControlMethod == 0) {
        QUIState::ui_state.scene.pidKi = QUIState::ui_state.scene.pidKi - 1;
        if (QUIState::ui_state.scene.pidKi <= 1) QUIState::ui_state.scene.pidKi = 1;
        QString value = QString::number(QUIState::ui_state.scene.pidKi);
        Params().put("PidKi", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 3 && QUIState::ui_state.scene.lateralControlMethod == 0) {
        QUIState::ui_state.scene.pidKd = QUIState::ui_state.scene.pidKd - 5;
        if (QUIState::ui_state.scene.pidKd <= 0) QUIState::ui_state.scene.pidKd = 0;
        QString value = QString::number(QUIState::ui_state.scene.pidKd);
        Params().put("PidKd", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 4 && QUIState::ui_state.scene.lateralControlMethod == 0) {
        QUIState::ui_state.scene.pidKf = QUIState::ui_state.scene.pidKf - 1;
        if (QUIState::ui_state.scene.pidKf <= 1) QUIState::ui_state.scene.pidKf = 1;
        QString value = QString::number(QUIState::ui_state.scene.pidKf);
        Params().put("PidKf", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 1 && QUIState::ui_state.scene.lateralControlMethod == 1) {
        QUIState::ui_state.scene.indiInnerLoopGain = QUIState::ui_state.scene.indiInnerLoopGain - 1;
        if (QUIState::ui_state.scene.indiInnerLoopGain <= 1) QUIState::ui_state.scene.indiInnerLoopGain = 1;
        QString value = QString::number(QUIState::ui_state.scene.indiInnerLoopGain);
        Params().put("InnerLoopGain", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 2 && QUIState::ui_state.scene.lateralControlMethod == 1) {
        QUIState::ui_state.scene.indiOuterLoopGain = QUIState::ui_state.scene.indiOuterLoopGain - 1;
        if (QUIState::ui_state.scene.indiOuterLoopGain <= 1) QUIState::ui_state.scene.indiOuterLoopGain = 1;
        QString value = QString::number(QUIState::ui_state.scene.indiOuterLoopGain);
        Params().put("OuterLoopGain", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 3 && QUIState::ui_state.scene.lateralControlMethod == 1) {
        QUIState::ui_state.scene.indiTimeConstant = QUIState::ui_state.scene.indiTimeConstant - 1;
        if (QUIState::ui_state.scene.indiTimeConstant <= 1) QUIState::ui_state.scene.indiTimeConstant = 1;
        QString value = QString::number(QUIState::ui_state.scene.indiTimeConstant);
        Params().put("TimeConstant", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 4 && QUIState::ui_state.scene.lateralControlMethod == 1) {
        QUIState::ui_state.scene.indiActuatorEffectiveness = QUIState::ui_state.scene.indiActuatorEffectiveness - 1;
        if (QUIState::ui_state.scene.indiActuatorEffectiveness <= 1) QUIState::ui_state.scene.indiActuatorEffectiveness = 1;
        QString value = QString::number(QUIState::ui_state.scene.indiActuatorEffectiveness);
        Params().put("ActuatorEffectiveness", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 1 && QUIState::ui_state.scene.lateralControlMethod == 2) {
        QUIState::ui_state.scene.lqrScale = QUIState::ui_state.scene.lqrScale - 50;
        if (QUIState::ui_state.scene.lqrScale <= 50) QUIState::ui_state.scene.lqrScale = 50;
        QString value = QString::number(QUIState::ui_state.scene.lqrScale);
        Params().put("Scale", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 2 && QUIState::ui_state.scene.lateralControlMethod == 2) {
        QUIState::ui_state.scene.lqrKi = QUIState::ui_state.scene.lqrKi - 1;
        if (QUIState::ui_state.scene.lqrKi <= 1) QUIState::ui_state.scene.lqrKi = 1;
        QString value = QString::number(QUIState::ui_state.scene.lqrKi);
        Params().put("LqrKi", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 3 && QUIState::ui_state.scene.lateralControlMethod == 2) {
        QUIState::ui_state.scene.lqrDcGain = QUIState::ui_state.scene.lqrDcGain - 5;
        if (QUIState::ui_state.scene.lqrDcGain <= 5) QUIState::ui_state.scene.lqrDcGain = 5;
        QString value = QString::number(QUIState::ui_state.scene.lqrDcGain);
        Params().put("DcGain", value.toStdString());
        return;
      }
    }
    if (QUIState::ui_state.scene.started && !sidebar->isVisible() && !QUIState::ui_state.scene.map_on_top && livetunepanel_right_btn.ptInRect(e->x(), e->y())) {
      if (QUIState::ui_state.scene.live_tune_panel_list == 0) {
        QUIState::ui_state.scene.cameraOffset = QUIState::ui_state.scene.cameraOffset + 5;
        if (QUIState::ui_state.scene.cameraOffset >= 1000) QUIState::ui_state.scene.cameraOffset = 1000;
        QString value = QString::number(QUIState::ui_state.scene.cameraOffset);
        Params().put("CameraOffsetAdj", value.toStdString());
        return;
      }
      if (QUIState::ui_state.scene.live_tune_panel_list == 1 && QUIState::ui_state.scene.lateralControlMethod == 0) {
        QUIState::ui_state.scene.pidKp = QUIState::ui_state.scene.pidKp + 1;
        if (QUIState::ui_state.scene.pidKp >= 50) QUIState::ui_state.scene.pidKp = 50;
        QString value = QString::number(QUIState::ui_state.scene.pidKp);
        Params().put("PidKp", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 2 && QUIState::ui_state.scene.lateralControlMethod == 0) {
        QUIState::ui_state.scene.pidKi = QUIState::ui_state.scene.pidKi + 1;
        if (QUIState::ui_state.scene.pidKi >= 100) QUIState::ui_state.scene.pidKi = 100;
        QString value = QString::number(QUIState::ui_state.scene.pidKi);
        Params().put("PidKi", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 3 && QUIState::ui_state.scene.lateralControlMethod == 0) {
        QUIState::ui_state.scene.pidKd = QUIState::ui_state.scene.pidKd + 5;
        if (QUIState::ui_state.scene.pidKd >= 300) QUIState::ui_state.scene.pidKd = 300;
        QString value = QString::number(QUIState::ui_state.scene.pidKd);
        Params().put("PidKd", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 4 && QUIState::ui_state.scene.lateralControlMethod == 0) {
        QUIState::ui_state.scene.pidKf = QUIState::ui_state.scene.pidKf + 1;
        if (QUIState::ui_state.scene.pidKf >= 50) QUIState::ui_state.scene.pidKf = 50;
        QString value = QString::number(QUIState::ui_state.scene.pidKf);
        Params().put("PidKf", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 1 && QUIState::ui_state.scene.lateralControlMethod == 1) {
        QUIState::ui_state.scene.indiInnerLoopGain = QUIState::ui_state.scene.indiInnerLoopGain + 1;
        if (QUIState::ui_state.scene.indiInnerLoopGain >= 200) QUIState::ui_state.scene.indiInnerLoopGain = 200;
        QString value = QString::number(QUIState::ui_state.scene.indiInnerLoopGain);
        Params().put("InnerLoopGain", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 2 && QUIState::ui_state.scene.lateralControlMethod == 1) {
        QUIState::ui_state.scene.indiOuterLoopGain = QUIState::ui_state.scene.indiOuterLoopGain + 1;
        if (QUIState::ui_state.scene.indiOuterLoopGain >= 200) QUIState::ui_state.scene.indiOuterLoopGain = 200;
        QString value = QString::number(QUIState::ui_state.scene.indiOuterLoopGain);
        Params().put("OuterLoopGain", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 3 && QUIState::ui_state.scene.lateralControlMethod == 1) {
        QUIState::ui_state.scene.indiTimeConstant = QUIState::ui_state.scene.indiTimeConstant + 1;
        if (QUIState::ui_state.scene.indiTimeConstant >= 200) QUIState::ui_state.scene.indiTimeConstant = 200;
        QString value = QString::number(QUIState::ui_state.scene.indiTimeConstant);
        Params().put("TimeConstant", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 4 && QUIState::ui_state.scene.lateralControlMethod == 1) {
        QUIState::ui_state.scene.indiActuatorEffectiveness = QUIState::ui_state.scene.indiActuatorEffectiveness + 1;
        if (QUIState::ui_state.scene.indiActuatorEffectiveness >= 200) QUIState::ui_state.scene.indiActuatorEffectiveness = 200;
        QString value = QString::number(QUIState::ui_state.scene.indiActuatorEffectiveness);
        Params().put("ActuatorEffectiveness", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 1 && QUIState::ui_state.scene.lateralControlMethod == 2) {
        QUIState::ui_state.scene.lqrScale = QUIState::ui_state.scene.lqrScale + 50;
        if (QUIState::ui_state.scene.lqrScale >= 5000) QUIState::ui_state.scene.lqrScale = 5000;
        QString value = QString::number(QUIState::ui_state.scene.lqrScale);
        Params().put("Scale", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 2 && QUIState::ui_state.scene.lateralControlMethod == 2) {
        QUIState::ui_state.scene.lqrKi = QUIState::ui_state.scene.lqrKi + 1;
        if (QUIState::ui_state.scene.lqrKi >= 100) QUIState::ui_state.scene.lqrKi = 100;
        QString value = QString::number(QUIState::ui_state.scene.lqrKi);
        Params().put("LqrKi", value.toStdString());
        return;
      } else if (QUIState::ui_state.scene.live_tune_panel_list == 3 && QUIState::ui_state.scene.lateralControlMethod == 2) {
        QUIState::ui_state.scene.lqrDcGain = QUIState::ui_state.scene.lqrDcGain + 5;
        if (QUIState::ui_state.scene.lqrDcGain >= 500) QUIState::ui_state.scene.lqrDcGain = 500;
        QString value = QString::number(QUIState::ui_state.scene.lqrDcGain);
        Params().put("DcGain", value.toStdString());
        return;
      }
    }
    if (QUIState::ui_state.scene.started && !sidebar->isVisible() && !QUIState::ui_state.scene.map_on_top && livetunepanel_left_above_btn.ptInRect(e->x(), e->y())) {
      QUIState::ui_state.scene.live_tune_panel_list = QUIState::ui_state.scene.live_tune_panel_list - 1;
      if (QUIState::ui_state.scene.lateralControlMethod == 2 && QUIState::ui_state.scene.live_tune_panel_list < 0) {
        QUIState::ui_state.scene.live_tune_panel_list = 3;
      } else if (QUIState::ui_state.scene.live_tune_panel_list < 0) {
        QUIState::ui_state.scene.live_tune_panel_list = 4;
      }
      return;
    }
    if (QUIState::ui_state.scene.started && !sidebar->isVisible() && !QUIState::ui_state.scene.map_on_top && livetunepanel_right_above_btn.ptInRect(e->x(), e->y())) {
      QUIState::ui_state.scene.live_tune_panel_list = QUIState::ui_state.scene.live_tune_panel_list + 1;
      if (QUIState::ui_state.scene.lateralControlMethod == 2 && QUIState::ui_state.scene.live_tune_panel_list > 3) {
        QUIState::ui_state.scene.live_tune_panel_list = 0;
      } else if (QUIState::ui_state.scene.live_tune_panel_list > 4) {
        QUIState::ui_state.scene.live_tune_panel_list = 0;
      }
      return;
    }
  }
  // Handle sidebar collapsing
  if (onroad->isVisible() && (!sidebar->isVisible() || e->x() > sidebar->width())) {

    // TODO: Handle this without exposing pointer to map widget
    // Hide map first if visible, then hide sidebar
    if (onroad->map != nullptr && onroad->map->isVisible()) {
      onroad->map->setVisible(false);
    } else if (!sidebar->isVisible()) {
      sidebar->setVisible(true);
    } else {
      sidebar->setVisible(false);

      if (onroad->map != nullptr) onroad->map->setVisible(true);
    }
    QUIState::ui_state.sidebar_view = !QUIState::ui_state.sidebar_view;
  }

  QUIState::ui_state.scene.setbtn_count = 0;
  QUIState::ui_state.scene.homebtn_count = 0;
  if (QUIState::ui_state.scene.started && QUIState::ui_state.scene.scr.autoScreenOff != -2) {
    QUIState::ui_state.scene.touched2 = true;
    QTimer::singleShot(500, []() { QUIState::ui_state.scene.touched2 = false; });
  }
}

// OffroadHome: the offroad home page

OffroadHome::OffroadHome(QWidget* parent) : QFrame(parent) {
  QVBoxLayout* main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(40, 40, 40, 45);

  // top header
  QHBoxLayout* header_layout = new QHBoxLayout();
  header_layout->setContentsMargins(15, 15, 15, 0);
  header_layout->setSpacing(16);

  date = new QLabel();
  header_layout->addWidget(date, 1, Qt::AlignHCenter | Qt::AlignLeft);

  update_notif = new QPushButton("UPDATE");
  update_notif->setVisible(false);
  update_notif->setStyleSheet("background-color: #364DEF;");
  QObject::connect(update_notif, &QPushButton::clicked, [=]() { center_layout->setCurrentIndex(1); });
  header_layout->addWidget(update_notif, 0, Qt::AlignHCenter | Qt::AlignRight);

  alert_notif = new QPushButton();
  alert_notif->setVisible(false);
  alert_notif->setStyleSheet("background-color: #E22C2C;");
  QObject::connect(alert_notif, &QPushButton::clicked, [=] { center_layout->setCurrentIndex(2); });
  header_layout->addWidget(alert_notif, 0, Qt::AlignHCenter | Qt::AlignRight);

  header_layout->addWidget(new QLabel(getBrandVersion()), 0, Qt::AlignHCenter | Qt::AlignRight);

  main_layout->addLayout(header_layout);

  // main content
  main_layout->addSpacing(25);
  center_layout = new QStackedLayout();

  QWidget* statsAndSetupWidget = new QWidget(this);
  QHBoxLayout* statsAndSetup = new QHBoxLayout(statsAndSetupWidget);
  statsAndSetup->setMargin(0);
  statsAndSetup->setSpacing(30);
  statsAndSetup->addWidget(new DriveStats, 1);
  statsAndSetup->addWidget(new SetupWidget);

  center_layout->addWidget(statsAndSetupWidget);

  // add update & alerts widgets
  update_widget = new UpdateAlert();
  QObject::connect(update_widget, &UpdateAlert::dismiss, [=]() { center_layout->setCurrentIndex(0); });
  center_layout->addWidget(update_widget);
  alerts_widget = new OffroadAlert();
  QObject::connect(alerts_widget, &OffroadAlert::dismiss, [=]() { center_layout->setCurrentIndex(0); });
  center_layout->addWidget(alerts_widget);

  main_layout->addLayout(center_layout, 1);

  // set up refresh timer
  timer = new QTimer(this);
  timer->callOnTimeout(this, &OffroadHome::refresh);

  setStyleSheet(R"(
    * {
     color: white;
    }
    OffroadHome {
      background-color: black;
    }
    OffroadHome > QPushButton {
      padding: 15px 30px;
      border-radius: 5px;
      font-size: 40px;
      font-weight: 500;
    }
    OffroadHome > QLabel {
      font-size: 55px;
    }
  )");
}

void OffroadHome::showEvent(QShowEvent *event) {
  refresh();
  timer->start(10 * 1000);
}

void OffroadHome::hideEvent(QHideEvent *event) {
  timer->stop();
}

void OffroadHome::refresh() {
  // opkr
  QLocale::setDefault(QLocale::Korean);
  QString date_kr = QDate::currentDate().toString(Qt::DefaultLocaleLongDate);
  QString time_kr = QTime::currentTime().toString(Qt::DefaultLocaleShortDate);
  date->setText(date_kr + " " + time_kr);

  bool updateAvailable = update_widget->refresh();
  int alerts = alerts_widget->refresh();

  // pop-up new notification
  int idx = center_layout->currentIndex();
  if (!updateAvailable && !alerts) {
    idx = 0;
  } else if (updateAvailable && (!update_notif->isVisible() || (!alerts && idx == 2))) {
    idx = 1;
  } else if (alerts && (!alert_notif->isVisible() || (!updateAvailable && idx == 1))) {
    idx = 2;
  }
  center_layout->setCurrentIndex(idx);

  update_notif->setVisible(updateAvailable);
  alert_notif->setVisible(alerts);
  if (alerts) {
    alert_notif->setText(QString::number(alerts) + " 경고" + (alerts > 1 ? "S" : ""));
  }
}
