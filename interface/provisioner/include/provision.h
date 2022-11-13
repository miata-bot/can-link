#pragma once

namespace Provision {
  void Init(int argc, char** argv);
  void Render(bool* requestDone);
  void Shutdown();
};