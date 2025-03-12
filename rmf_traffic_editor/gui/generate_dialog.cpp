#include <QtWidgets>
#include <future>

#include "generate_dialog.h"
#include "floorplan_annotator/utils/curl_communicator.hpp"

GenerateDialog::GenerateDialog(QWidget* parent, Building& _building)
: building(_building)
{
  setWindowTitle("Vertices and Lane Generation");
  const int reference_level_idx = building.get_reference_level_idx();
  ok_button = new QPushButton("OK", this);
  cancel_button = new QPushButton("Cancel", this);
  QHBoxLayout* bottom_buttons_hbox = new QHBoxLayout;
  bottom_buttons_hbox->addWidget(cancel_button);
  bottom_buttons_hbox->addWidget(ok_button);
  
  QHBoxLayout* model = new QHBoxLayout;
  model->addWidget(new QLabel("Floorplan Anaylsis Model parameters:"));
  infile_edit = new QLineEdit(
    QString::fromStdString(building.levels[reference_level_idx].drawing_filename), this);
  input_filename_button = new QPushButton("Find...", this);
  QHBoxLayout* infile_hbox = new QHBoxLayout;
  infile_hbox->addWidget(new QLabel("input file path:"));
  infile_hbox->addWidget(infile_edit);
  infile_hbox->addWidget(input_filename_button);
  connect(
    input_filename_button,
    &QAbstractButton::clicked,
    this,
    &GenerateDialog::input_filename_button_clicked);
  
  outfile_edit = new QLineEdit(
    QString::fromStdString("output.png"), this);
  output_filename_button = new QPushButton("Find...", this);
  QHBoxLayout* outfile_hbox = new QHBoxLayout;
  outfile_hbox->addWidget(new QLabel("output file path:"));
  outfile_hbox->addWidget(outfile_edit);
  outfile_hbox->addWidget(output_filename_button);
  connect(
    output_filename_button,
    &QAbstractButton::clicked,
    this,
    &GenerateDialog::output_filename_button_clicked);
  
  colorize_box = new QCheckBox("colorize", this);
  postprocess_box = new QCheckBox("postprocess", this);
  colorize_box->setChecked(false);
  postprocess_box->setChecked(false);
  QHBoxLayout* colorize_postprocess= new QHBoxLayout;
  colorize_postprocess->addWidget(colorize_box);
  colorize_postprocess->addWidget(postprocess_box);

  infer_button = new QPushButton("Perform model inference", this);
  QHBoxLayout* infer_hbox = new QHBoxLayout;
  infer_hbox->addWidget(infer_button);
  connect(
    infer_button,
    &QAbstractButton::clicked,
    this,
    &GenerateDialog::inference_button_clicked);

  QHBoxLayout* vertices_button_hbox = new QHBoxLayout;
  vertices_button = new QPushButton("Generate vertices", this);
  vertices_button_hbox->addWidget(vertices_button);

  QHBoxLayout* pixel_dist = new QHBoxLayout;
  pixel_dist->addWidget(new QLabel("x,y pixel distance between each vertex"));

  QHBoxLayout* x_hbox = new QHBoxLayout;
  x_pixels = new QLineEdit(QString::number(8), this);
  x_hbox->addWidget(new QLabel("x pixel distance"));
  x_hbox->addWidget(x_pixels);

  QHBoxLayout* y_hbox = new QHBoxLayout;
  y_pixels = new QLineEdit(QString::number(8), this);
  y_hbox->addWidget(new QLabel("y pixel distance"));
  y_hbox->addWidget(y_pixels);

  connect(
    cancel_button,
    &QAbstractButton::clicked,
    this,
    &QDialog::reject);
  
    connect(
      vertices_button,
      &QAbstractButton::clicked,
      this,
      &GenerateDialog::vertex_gen_button_clicked);

  // formatting QDialog box  
  QVBoxLayout* top_vbox = new QVBoxLayout;
  top_vbox->addLayout(model);
  top_vbox->addLayout(infile_hbox);
  top_vbox->addLayout(outfile_hbox);
  top_vbox->addLayout(colorize_postprocess);
  top_vbox->addLayout(infer_hbox);
  top_vbox->addLayout(x_hbox);
  top_vbox->addLayout(y_hbox);
  top_vbox->addLayout(pixel_dist);
  top_vbox->addLayout(vertices_button_hbox);
  top_vbox->addLayout(bottom_buttons_hbox);
  setLayout(top_vbox);
}

void GenerateDialog::input_filename_button_clicked()
{
  QFileDialog file_dialog(this, "Find Drawing");
  file_dialog.setFileMode(QFileDialog::ExistingFile);
  file_dialog.setNameFilter("*.jpg");
  if (file_dialog.exec() != QDialog::Accepted)
  {
    if (infile_edit->text().isEmpty()) {

    }
    return;  // user clicked 'cancel'
  }
  const QString filename = file_dialog.selectedFiles().first();
  if (!filename.endsWith(".jpg"))
  {
    QMessageBox::critical(
      this,
      "Incorrect suffix",
      "Incorrect suffix! File should contain the .jpg extension for inference" );
  }
  if (!QFileInfo(filename).exists())
  {
    QMessageBox::critical(
      this,
      "Image file does not exist",
      "Image file does not exist.");
    return;
  }
  infile_edit->setText(
    QDir::current().absoluteFilePath(filename));
}

void GenerateDialog::output_filename_button_clicked()
{
  QFileDialog file_dialog(this, "Find Drawing");
  file_dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (file_dialog.exec() != QDialog::Accepted)
  {
    return;  // user clicked 'cancel'
  }
  const QString filepath = file_dialog.selectedFiles().first();
  if (!filepath.endsWith(".png"))
  {
    QMessageBox::critical(
      this,
      "Incorrect suffix",
      "Incorrect suffix! Path should contain absolute path including desired output filename eg. ~/ws/output.png");
    return;
  }
  
  outfile_edit->setText(
    QDir::current().absoluteFilePath(filepath));
}

void GenerateDialog::inference_button_clicked()
{
  auto color = colorize_box->checkState() == Qt::Unchecked ? 0 : 1;
  auto postprocess = postprocess_box->checkState() == Qt::Unchecked ? 0 : 1;
  auto http_ptr = std::make_unique<floorplan_annotator::utils::CurlCommunicator>();
  auto state = std::async(std::launch::async, &floorplan_annotator::utils::CurlCommunicator::post_request, std::move(http_ptr), 
    infile_edit->displayText().toStdString(), outfile_edit->displayText().toStdString(), 
    color, postprocess);
  auto curl_result = state.get();
  if (curl_result == CurlStatus::OK)
  {
    QMessageBox::information(
      this,
      "Succeed",
      "Inference success!");
  }
  else if (curl_result == CurlStatus::ERROR)
  {
    QMessageBox::critical(
      this,
      "Failed",
      "Inference failed! Check if the model's docker container is running!");
  }
}

void GenerateDialog::vertex_gen_button_clicked()
{
  if (!file_exists(outfile_edit->displayText()))
  {
    QMessageBox::critical(
      this,
      "Failed",
      "Invalid output file path! Unable to generate vertices!");
    return;
  }
  if (x_pixels->displayText().toDouble() == 0 || y_pixels->displayText().toDouble() == 0) 
  {
    QMessageBox::critical(
      this,
      "Failed",
      "Invalid pixel size!");
    return;
  }
  accept();
}

double GenerateDialog::get_x_pixel_dist() const
{
  return x_pixels->displayText().toDouble();
}

double GenerateDialog::get_y_pixel_dist() const
{
  return y_pixels->displayText().toDouble();
}

std::string GenerateDialog::get_output_filepath() const
{
  return outfile_edit->displayText().toStdString();
}

bool GenerateDialog::file_exists(const QString & path)
{
  QFileInfo file(path);
  return file.exists() && file.isFile();
}

GenerateDialog::~GenerateDialog()
{

}