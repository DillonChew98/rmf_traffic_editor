#ifndef GENERATE_DIALOG_H
#define GENERATE_DIALOG_H

#include <QDialog>
#include <QObject>
#include <QCheckBox>

#include "building.h"
#include "editor_model.h"

#include <vector>
#include <memory>
#include <string>
#include <utility>


class QLineEdit;
class QListWidget;
class QLabel;

class GenerateDialog : public QDialog
{
public:
  GenerateDialog(QWidget* parent, Building& building);

  ~GenerateDialog();

  std::string get_output_filepath() const;

  double get_x_pixel_dist() const;

  double get_y_pixel_dist() const;

private:
  bool file_exists(const QString & path);

  Building& building;

  QLineEdit* infile_edit, * outfile_edit;

  QLineEdit* x_pixels, * y_pixels;

  QCheckBox* postprocess_box, * colorize_box;

  QPushButton* input_filename_button, * output_filename_button;

  QPushButton* ok_button, * cancel_button, * infer_button;

  QPushButton* vertices_button;

private slots:
  void input_filename_button_clicked();

  void output_filename_button_clicked();

  void inference_button_clicked();

  void vertex_gen_button_clicked();

  void drawing_filename_line_edited(const QString& text);
};

#endif