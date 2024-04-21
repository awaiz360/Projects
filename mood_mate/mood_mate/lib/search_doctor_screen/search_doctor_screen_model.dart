import '/flutter_flow/flutter_flow_util.dart';
import 'search_doctor_screen_widget.dart' show SearchDoctorScreenWidget;
import 'package:flutter/material.dart';

class SearchDoctorScreenModel
    extends FlutterFlowModel<SearchDoctorScreenWidget> {
  ///  State fields for stateful widgets in this page.

  final unfocusNode = FocusNode();
  // State field(s) for TextField widget.
  FocusNode? textFieldFocusNode;
  TextEditingController? textController;
  String? Function(BuildContext, String?)? textControllerValidator;

  @override
  void initState(BuildContext context) {}

  @override
  void dispose() {
    unfocusNode.dispose();
    textFieldFocusNode?.dispose();
    textController?.dispose();
  }
}
