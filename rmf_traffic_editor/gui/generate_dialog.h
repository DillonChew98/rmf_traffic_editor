#ifndef GENERATE_DIALOG_H
#define GENERATE_DIALOG_H

#include <QDialog>
#include <QObject>
#include <QCheckBox>

#include "level.h"
#include "building.h"
#include "editor_model.h"

#include <vector>
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

private:
  Building& building;
  
  QLineEdit* infile_edit, * outfile_edit;
  QCheckBox* postprocess_box, * colorize_box;
  QPushButton* input_filename_button, * output_filename_button;
  QPushButton* ok_button, * cancel_button, * infer_button;

private slots:
  void input_filename_button_clicked();
  void output_filename_button_clicked();
  void inference_button_clicked();
  void drawing_filename_line_edited(const QString& text);
};

#endif