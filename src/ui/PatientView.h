/**
 * @file    PatientView.h
 * @brief   Widget chính hiển thị và quản lý danh sách bệnh nhân.
 */
#pragma once

#include <QWidget>

class QTableView;
class QLineEdit;
class QPushButton;
class PatientTableModel;
class PatientService;

/**
 * @brief Trang chính của module Patient.
 *
 * Chứa QTableView + thanh tìm kiếm + các nút Thêm/Sửa/Xóa.
 * Kết nối UI events → gọi PatientService → cập nhật PatientTableModel.
 */
class PatientView : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Khởi tạo PatientView với service.
     * @param service Con trỏ đến PatientService (không sở hữu).
     * @param parent Widget cha.
     */
    explicit PatientView(PatientService* service,
                         QWidget* parent = nullptr);
    ~PatientView() override = default;

private slots:
    void handleAddClicked();
    void handleEditClicked();
    void handleDeleteClicked();
    void handleSearchClicked();
    void handleSearchTextChanged(const QString& text);

private:
    void setupUi();
    void refreshTable();

    /**
     * @brief Lấy row index đang được chọn trong table.
     * @return Row index (0-indexed), hoặc -1 nếu chưa chọn.
     */
    int selectedRow() const;

    PatientService*    m_service;
    PatientTableModel* m_model;
    QTableView*        m_tableView;
    QLineEdit*         m_searchInput;
    QPushButton*       m_btnSearch;
    QPushButton*       m_btnAdd;
    QPushButton*       m_btnEdit;
    QPushButton*       m_btnDelete;
};
