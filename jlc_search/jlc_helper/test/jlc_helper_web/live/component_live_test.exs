defmodule JlcHelperWeb.ComponentLiveTest do
  use JlcHelperWeb.ConnCase

  import Phoenix.LiveViewTest
  import JlcHelper.JLCFixtures

  @create_attrs %{category1: "some category1", category2: "some category2", datasheet: "some datasheet", description: "some description", joins: 42, library_type: "some library_type", manufacturer: "some manufacturer", mpn: "some mpn", package: "some package", part: "some part", price: 120.5, stock: 42}
  @update_attrs %{category1: "some updated category1", category2: "some updated category2", datasheet: "some updated datasheet", description: "some updated description", joins: 43, library_type: "some updated library_type", manufacturer: "some updated manufacturer", mpn: "some updated mpn", package: "some updated package", part: "some updated part", price: 456.7, stock: 43}
  @invalid_attrs %{category1: nil, category2: nil, datasheet: nil, description: nil, joins: nil, library_type: nil, manufacturer: nil, mpn: nil, package: nil, part: nil, price: nil, stock: nil}

  defp create_component(_) do
    component = component_fixture()
    %{component: component}
  end

  describe "Index" do
    setup [:create_component]

    test "lists all componens", %{conn: conn, component: component} do
      {:ok, _index_live, html} = live(conn, Routes.component_index_path(conn, :index))

      assert html =~ "Listing Componens"
      assert html =~ component.category1
    end

    test "saves new component", %{conn: conn} do
      {:ok, index_live, _html} = live(conn, Routes.component_index_path(conn, :index))

      assert index_live |> element("a", "New Component") |> render_click() =~
               "New Component"

      assert_patch(index_live, Routes.component_index_path(conn, :new))

      assert index_live
             |> form("#component-form", component: @invalid_attrs)
             |> render_change() =~ "can&#39;t be blank"

      {:ok, _, html} =
        index_live
        |> form("#component-form", component: @create_attrs)
        |> render_submit()
        |> follow_redirect(conn, Routes.component_index_path(conn, :index))

      assert html =~ "Component created successfully"
      assert html =~ "some category1"
    end

    test "updates component in listing", %{conn: conn, component: component} do
      {:ok, index_live, _html} = live(conn, Routes.component_index_path(conn, :index))

      assert index_live |> element("#component-#{component.id} a", "Edit") |> render_click() =~
               "Edit Component"

      assert_patch(index_live, Routes.component_index_path(conn, :edit, component))

      assert index_live
             |> form("#component-form", component: @invalid_attrs)
             |> render_change() =~ "can&#39;t be blank"

      {:ok, _, html} =
        index_live
        |> form("#component-form", component: @update_attrs)
        |> render_submit()
        |> follow_redirect(conn, Routes.component_index_path(conn, :index))

      assert html =~ "Component updated successfully"
      assert html =~ "some updated category1"
    end

    test "deletes component in listing", %{conn: conn, component: component} do
      {:ok, index_live, _html} = live(conn, Routes.component_index_path(conn, :index))

      assert index_live |> element("#component-#{component.id} a", "Delete") |> render_click()
      refute has_element?(index_live, "#component-#{component.id}")
    end
  end

  describe "Show" do
    setup [:create_component]

    test "displays component", %{conn: conn, component: component} do
      {:ok, _show_live, html} = live(conn, Routes.component_show_path(conn, :show, component))

      assert html =~ "Show Component"
      assert html =~ component.category1
    end

    test "updates component within modal", %{conn: conn, component: component} do
      {:ok, show_live, _html} = live(conn, Routes.component_show_path(conn, :show, component))

      assert show_live |> element("a", "Edit") |> render_click() =~
               "Edit Component"

      assert_patch(show_live, Routes.component_show_path(conn, :edit, component))

      assert show_live
             |> form("#component-form", component: @invalid_attrs)
             |> render_change() =~ "can&#39;t be blank"

      {:ok, _, html} =
        show_live
        |> form("#component-form", component: @update_attrs)
        |> render_submit()
        |> follow_redirect(conn, Routes.component_show_path(conn, :show, component))

      assert html =~ "Component updated successfully"
      assert html =~ "some updated category1"
    end
  end
end
