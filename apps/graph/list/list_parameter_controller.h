#ifndef GRAPH_LIST_LIST_PARAM_CONTROLLER_H
#define GRAPH_LIST_LIST_PARAM_CONTROLLER_H

#include <apps/shared/list_parameter_controller.h>
#include <escher/message_table_cell_with_chevron_and_message.h>
#include <escher/message_table_cell_with_chevron_and_buffer.h>
#include "type_parameter_controller.h"
#include "domain_parameter_controller.h"

namespace Graph {

class ListController;

class ListParameterController : public Shared::ListParameterController {
public:
  ListParameterController(ListController * listController, Escher::Responder * parentResponder, I18n::Message functionColorMessage, I18n::Message deleteFunctionMessage, Escher::InputEventHandlerDelegate * inputEventHandlerDelegate);
  bool handleEvent(Ion::Events::Event event) override;
  // ListViewDataSource
  KDCoordinate rowHeight(int j) override;
  Escher::HighlightCell * reusableCell(int index, int type) override;
  void willDisplayCellForIndex(Escher::HighlightCell * cell, int index) override;
protected:
  bool handleEnterOnRow(int rowIndex) override;
private:
  int totalNumberOfCells() const override {
    return Shared::ListParameterController::totalNumberOfCells() + 2;
  }
  void renameFunction();
  ListController * m_listController;
  Escher::MessageTableCellWithChevronAndMessage m_typeCell;
  Escher::MessageTableCellWithChevronAndBuffer m_functionDomain;
  TypeParameterController m_typeParameterController;
  DomainParameterController m_domainParameterController;
  Escher::MessageTableCell m_renameCell;
};

}

#endif
