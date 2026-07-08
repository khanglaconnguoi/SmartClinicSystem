#pragma once

#include "../BaseDashboard.h"
#include <memory>
#include <QString>

class IAuthenticatable;
class QStackedWidget;
class QPushButton;
class QWidget;
class QVBoxLayout;
class QHBoxLayout;
class QComboBox;
class QDateEdit;
class QTableWidget;

class ReceptionDashboardWidget : public BaseDashboardWidget {
    Q_OBJECT

public:
    explicit ReceptionDashboardWidget(std::shared_ptr<IAuthenticatable> user, QWidget *parent = nullptr);
    virtual ~ReceptionDashboardWidget() override = default;

protected:
    virtual void fillDashboardData() override;

private:
    std::shared_ptr<IAuthenticatable> m_currentUser;

    // Sidebar buttons and stacked widget
    QStackedWidget* m_stackedWidget = nullptr;
    QPushButton*    m_btnOverview   = nullptr;
    QPushButton*    m_btnRegister   = nullptr;
    QPushButton*    m_btnPatients   = nullptr;

    // Pages
    QWidget*        m_overviewPage   = nullptr;
    QWidget*        m_registerPage   = nullptr;
    QWidget*        m_patientsPage   = nullptr;

    // Registration UI elements
    class QFrame*   m_apptCard       = nullptr;
    QLineEdit* m_txtPatientPhone = nullptr;
    QLineEdit* m_txtPatientCitizenId  = nullptr;
    QPushButton*    m_btnContinue    = nullptr;
    QPushButton*    m_btnConfirm     = nullptr;
    int             m_currentPatientId = -1;

    QComboBox*      m_comboSpecialty = nullptr;
    QComboBox*      m_comboDoctor    = nullptr;
    QDateEdit*      m_dateEdit       = nullptr;
    QTableWidget*   m_timeSlotTable  = nullptr;
    
    // Overview UI elements
    class QLabel*   m_lblRevenue     = nullptr;
    class QLabel*   m_lblPatientNum  = nullptr;

    // Build functions
    void buildSidebar();
    void buildOverviewPage();
    void buildRegisterPage();
    void buildPatientsPage();

    class QFrame* makeCard(QWidget* parent = nullptr);

    // Helpers
    void switchPage(int index, QPushButton* activeBtn);
    void loadAvailableTimeSlots();
    void loadDoctorsBySpecialty(const QString& specialty);
    
    // Slots
    void onContinueClicked();
    void onConfirmClicked();
};
