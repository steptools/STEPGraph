#include <rose.h>
#include <stp_schema.h>

using namespace System;
void markme(RoseObject * obj);
void graphaddnode(Microsoft::Msagl::Drawing::Graph^ graph, String^ out, String^ in, String^ name="");
void graphatts(RoseObject* obj, Microsoft::Msagl::Drawing::Graph^ graph);
void parsetonode(RoseObject*parent, RoseObject*child, String^ attstr, Microsoft::Msagl::Drawing::Graph^ graph);
int main(int argc, char ** argv)
{
    if (argc < 3)
    {
	Console::WriteLine("Usage: STEPVis.exe [step file to map] [depth]");
	return -1;
    }
    int depth;
    try
    {
	String ^ depthstr = gcnew String(argv[2]);
	depth = Convert::ToInt32(depthstr);
    }
    catch (Exception^)
    {
	Console::WriteLine("invalid depth parameter.");
	return -1;
    }

    ROSE.quiet(ROSE_TRUE);
    FILE * f = fopen("rlog.txt","w");
    if (nullptr == f) return -1;
    ROSE.error_reporter()->error_file(f);
    stplib_init();
    System::Windows::Forms::Form^ form = gcnew System::Windows::Forms::Form();
    Microsoft::Msagl::GraphViewerGdi::GViewer^ viewer = gcnew Microsoft::Msagl::GraphViewerGdi::GViewer();
    Microsoft::Msagl::Drawing::Graph^ graph = gcnew Microsoft::Msagl::Drawing::Graph("graph");
    RoseDesign * d = ROSE.findDesign(argv[1]);
    if (!d)
    {
	printf("Usage: %s [step file to map]", argv[0]);
	return -1;
    }
    RoseCursor curs;
    curs.traverse(d);
    curs.domain(ROSE_DOMAIN(stp_draughting_model));
    RoseObject * obj = curs.next();
    if (obj == nullptr)
	return -1;
    printf("Got a %s\n", obj->domain()->name());
    ListOfRoseObject lst;
    obj->findObjects(&lst, depth, false);
    lst.add(obj);
    RoseMark marker= rose_mark_begin();
    //rose_compute_backptrs(d);
    for (unsigned i = 0, sz = lst.size(); i < sz;i++)
    {
	obj = lst[i];
	if (nullptr == obj) continue;
	if (obj->isa(ROSE_DOMAIN(RoseUnion)))
	{
	    obj = rose_get_nested_object(ROSE_CAST(RoseUnion, obj));
	    if (nullptr == obj) continue; //Select with a non-object type
	}
	if (obj->isa(ROSE_DOMAIN(RoseAggregate)))
	{
	    for (unsigned j = 0, jsz = obj->size(); j < jsz; j++)
	    {
		if (obj->getObject(j) == nullptr) graphatts(obj,graph);//not an aggregate of objects.
		else graphatts(obj->getObject(j),graph);
	    }
	}
	if (obj->isa(ROSE_DOMAIN(RoseStructure))) graphatts(obj,graph);
	//if (rose_is_marked(obj)) continue;
	//markme(obj);
	/*auto eid = obj->entity_id();
	for (auto i = 0u, sz = obj->attributes()->size(); i < sz; i++)
	{
	    auto sobj = obj->getObject(obj->attributes()->get(i));
	    if (nullptr == sobj) continue;
	    auto seid = sobj->entity_id();
	    if (seid != 0)
		graphaddnode(graph, gcnew String(obj->domain()->name()), gcnew String(sobj->domain()->name()), gcnew String(obj->attributes()->get(i)->name()));
	}
	if (false)//(obj->domain()->typeIsEntity())
	{
	    RoseBackptrs *rbp = rose_get_backptrs(obj);
	    for (auto i = 0u, sz = rbp->size(); i < sz; i++)
	    {
		auto seid = rbp->get(i)->entity_id();
		if (seid != 0)
		{
		    //for ()
		    //graphaddnode(graph, eid, seid, );
		}
	    }
	}*/
    }
    viewer->Graph = graph;
    //associate the viewer with the form 
    form->SuspendLayout();
    viewer->Dock = System::Windows::Forms::DockStyle::Fill;
    form->Controls->Add(viewer);
    form->ResumeLayout();
    //show the form 
    form->ShowDialog();
    return 0;
}

void graphatts(RoseObject* obj,Microsoft::Msagl::Drawing::Graph^ graph)
{
    if (obj->isa(ROSE_DOMAIN(stp_draughting_model)))
    {
	while (0);
    }
    printf("Getting atts of Obj %s\n", obj->domain()->name());
    auto eid = obj->entity_id();
    for (auto i = 0u, sz = obj->attributes()->size(); i < sz; i++)
    {
	auto sobj = obj->getObject(obj->attributes()->get(i));
	if (nullptr == sobj)
	{
	    printf("\t att %d: %s\n", i, obj->attributes()->get(i)->name());
	    graphaddnode(graph, gcnew String(obj->domain()->name()), gcnew String(obj->attributes()->get(i)->name()), gcnew String(obj->attributes()->get(i)->name()));
	    continue;
	}
	printf("\t att %d: %s\n", i, sobj->domain()->name());
	parsetonode(obj, sobj, gcnew String(obj->attributes()->get(i)->name()), graph);
    }
}

void parsetonode(RoseObject*parent, RoseObject*child,String ^ attstr, Microsoft::Msagl::Drawing::Graph^ graph)
{
    if (!child || !parent) return;
    if (child->isa(ROSE_DOMAIN(RoseUnion)))
    {
	RoseObject * unnestchild = rose_get_nested_object(ROSE_CAST(RoseUnion, child));
	graphaddnode(graph, gcnew String(parent->domain()->name()), gcnew String(child->domain()->name()), attstr);
	if (nullptr == unnestchild)
	    return;
	else
	{
	    parsetonode(child, unnestchild, gcnew String(""), graph);
	    return;
	}
    }
    if (child->isa(ROSE_DOMAIN(RoseAggregate)))
    {
	graphaddnode(graph, gcnew String(parent->domain()->name()), gcnew String(child->domain()->name()), attstr);
	for (unsigned i = 0, sz = child->size(); i < sz; i++)
	{
	    if (nullptr == child->getObject(i)) return;
	    parsetonode(child,child->getObject(i),gcnew String(child->getAttribute()->name()),graph);
	}
    }
    if (child->isa(ROSE_DOMAIN(RoseStructure)))
	graphaddnode(graph, gcnew String(parent->domain()->name()), gcnew String(child->domain()->name()), attstr);
    
}

void graphaddnode(Microsoft::Msagl::Drawing::Graph^ graph, String^ out, String^ in,String^ name)
{
    if (in == "name")
	in = out + "_name";
    Microsoft::Msagl::Drawing::Node^ nout = graph->FindNode(out);
    Microsoft::Msagl::Drawing::Node^ nin = graph->FindNode(in);
    if (!nout)
    {
	nout = graph->AddNode(out);
    }
    if (!nin)
    {
	nin = graph->AddNode(in);
    }
    for each (auto edge in nout->Edges)
    {
	if (edge->TargetNode == nin && edge->SourceNode == nout) return;
    }
    Microsoft::Msagl::Drawing::Edge ^ edge = gcnew Microsoft::Msagl::Drawing::Edge(nout,nin,Microsoft::Msagl::Drawing::ConnectionToGraph::Connected);
    edge->LabelText = name;
    if (in == out + "_name")
	nin->LabelText = "name";
    return;
}

void markme(RoseObject*obj)
{
    if (nullptr == obj) return;
    if (rose_is_marked(obj)) return;
    //MARK HERE
    rose_mark_set(obj);
    auto foo = obj->attributes();
    if (obj->isa(ROSE_DOMAIN(RoseStructure)))
    {
	for (auto i = 0u; i < foo->size(); i++)
	{
	    markme(obj->getObject(foo->get(i)));
	}
    }
    if (obj->isa(ROSE_DOMAIN(RoseUnion)))
    {
	markme(rose_get_nested_object(ROSE_CAST(RoseUnion,obj)));
    }
    if (obj->isa(ROSE_DOMAIN(RoseAggregate)))
    {
	RoseAttribute *att = obj->getAttribute();
	if (!att->isObject()) return;
	for (auto i = 0u, sz = obj->size(); i < sz;i++)
	{
	    markme(obj->getObject(i));
	}
    }
}