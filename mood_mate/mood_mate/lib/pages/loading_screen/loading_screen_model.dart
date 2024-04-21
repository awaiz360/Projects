import '/flutter_flow/flutter_flow_util.dart';
import 'loading_screen_widget.dart' show LoadingScreenWidget;
import 'package:flutter/material.dart';

class LoadingScreenModel extends FlutterFlowModel<LoadingScreenWidget> {
  ///  State fields for stateful widgets in this page.

  final unfocusNode = FocusNode();

  @override
  void initState(BuildContext context) {}

  @override
  void dispose() {
    unfocusNode.dispose();
  }
}
