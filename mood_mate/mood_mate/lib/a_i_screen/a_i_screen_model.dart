import '/backend/api_requests/api_calls.dart';
import '/flutter_flow/flutter_flow_util.dart';
import 'a_i_screen_widget.dart' show AIScreenWidget;
import 'package:flutter/material.dart';

class AIScreenModel extends FlutterFlowModel<AIScreenWidget> {
  ///  State fields for stateful widgets in this page.

  final unfocusNode = FocusNode();
  // State field(s) for quesTxt widget.
  FocusNode? quesTxtFocusNode;
  TextEditingController? quesTxtTextController;
  String? Function(BuildContext, String?)? quesTxtTextControllerValidator;
  // Stores action output result for [Backend Call - API (aiBotAPI)] action in Button widget.
  ApiCallResponse? apiResultegt;

  @override
  void initState(BuildContext context) {}

  @override
  void dispose() {
    unfocusNode.dispose();
    quesTxtFocusNode?.dispose();
    quesTxtTextController?.dispose();
  }
}
